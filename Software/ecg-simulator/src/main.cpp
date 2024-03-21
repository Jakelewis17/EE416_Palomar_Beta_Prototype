#include <Arduino.h>

//#define ENABLE_MODE_SELECTOR
#define ENABLE_RESP_LED
//define SPEED_8_MHZ
#define PINOUT 9
#ifndef SPEED_8_MHZ
#define F_CPU 16000000UL
#else
#define F_CPU 8000000UL
#endif

// Timings
enum hr_rate{
  BPM40 = 0x3C,
  BPM80 = 0x1E,
  BPM120 = 0x14,
  TACH = 0xF,
};

// Respiratory Waveform Length
#ifndef SPEED_8_MHZ
enum resp_rate {
  RESP12 = 150,
  RESP38 = 50,
};
#else
enum resp_rate {
  RESP12 = 75,
  RESP38 = 25,
};
#endif

// Waveform Arrays
// QRS complex and array with baseline offset
uint8_t nsr_fragment[] = {
  35, 38, 35, 20, 20, 20, 25, 5, 
  140, 255, 0, 20, 20, 20, 25,
  35, 45, 55, 58, 56, 25
};
uint8_t pwm_norm_sr[0x40];
// ventricular tachycardia
uint8_t pwm_vtach[0x10] {
  0, 100, 150, 200, 220, 240, 250, 240,
  255, 210, 180, 140, 100, 80, 40, 10
};
// ventricular fibrillation - dynamically generated
uint8_t pwm_vfib[0x40];

// Functions
#ifdef ENABLE_MODE_SELECTOR
// For whatever reason, the optimizer doesn't always inline these function
// Force inlining with the __attribute__ modifier
// PORTD 2,3,4
uint8_t __attribute__((always_inline)) get_mode(void) {
  return((PIND >> 2) & 0x7);
}
#endif

void __attribute__((always_inline)) disable_resp(void) {
  TCCR2B = 0;
}

void __attribute__((always_inline)) enable_resp(void) {
  TCCR2B = 0x7;   // (clk/1024) prescalar
}

// Since there is only one PWM pin used that has been pre-set in setup(),
// this routine is faster than calling analogWrite()
void __attribute__((always_inline)) pwm_dc(const uint8_t duty_cycle) {
  OCR1A = duty_cycle;
}

void __attribute__((hot)) pwm_array_sequence(const uint8_t *const sequence_array, const uint8_t rate) {
  static uint8_t counter;
  uint8_t value = sequence_array[counter];
  counter = (counter + 1) % rate;
  pwm_dc(value);
  PORTD = PORTD & ~0x20;
  if(value == 255)
    PORTD = PORTD | (1 << PD5);
}

// This routine controls the respiratory simulation. Unfortunately, TIMER2 must be
// used as TIMER1 is governing the PWM signal. Even with the highest prescalar, the 
// timer overflows too quickly. So, we have to use a second counter to further 
// divide the signal down to a reasonable rate. The interrupt flag is cleared in 
// hardware when this routine is called.
static long resp_rate = RESP12;
ISR(TIMER2_OVF_vect) {
  static uint8_t counter = 0;
  counter = (counter + 1) % resp_rate;
  if(counter == 0) {
    DDRB = DDRB ^ 0x4;
    PORTB = PORTB & ~0x4;
#ifdef ENABLE_RESP_LED
    PORTB = (PORTB ^ 0x1);
#endif 
  }
}


// Setup
// Since this function is only called once, we can use the Arduino-core functions
// Arduino-core functions should be avoided when optimizing for speed.
// NOTE: this microcontroller must have at least three, independent interrupt timers
// TIMER0 is reserved for the delay function and can't be used.
void setup(void) {
  pinMode(5, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(PINOUT, OUTPUT);
#ifdef ENABLE_RESP_LED
  pinMode(8, OUTPUT);
#endif

  // Enable non-inverted, high-speed PWM for pin 9 (PB1) on TIMER1
  // OCR1A register determines duty cycle in this mode (range 0 - 255)
  // Registers and constant values taken from ATMega328P datasheet - timer control registers
  cli();
  TCCR1A |= _BV(COM1A1) | _BV(WGM10);
  TCCR1B |= _BV(CS10) | _BV(WGM12);

  // Enable TIMER2 overflow interrupt for respiration routine
  // Allows the respiratory simulation to run asynchronously to the cardiac waveforms
  TCCR2A = 0;
  enable_resp();
  TCNT2 = 0;
  TIMSK2 = _BV(TOIE2);  // enable TIMER2 overflow interrupt
  sei();

  // Precompute vfib values
  // This trig algorithm roughly simulates the random-yet-cyclical nature
  // of V-FIB. Enough to trigger the V-FIB alarms, usually. It will still 
  // occasionally be seen as a V-RUN.
  for(uint8_t i = 0; i < sizeof(pwm_vfib); i++)
    pwm_vfib[i] = (50 * (sin(i / 3) * sin((i / 3) * 2))) + 50;
  
  // Create normal sinus rhythm sequence from nsr_fragment and baseline offset
  for(uint8_t i = 0; i < sizeof(pwm_norm_sr); i++)
    pwm_norm_sr[i] = 20;
  for(uint8_t i = 0; i < sizeof(nsr_fragment); i++)
    pwm_norm_sr[i] = nsr_fragment[i];
}


// Loop
void __attribute__((hot)) loop(void) {
  uint8_t *current_sequence = pwm_norm_sr;
  //current_sequence = pwm_vtach;
  //resp_rate = RESP38;
  uint8_t heart_rate = BPM80;
  uint8_t master_delay = 25;
  uint8_t current_mode = 0;
  while(1) {
    pwm_array_sequence(current_sequence, heart_rate);
#ifdef ENABLE_MODE_SELECTOR
    uint8_t mode = get_mode();
    if(mode != current_mode) {
      current_mode = mode;
      current_sequence = pwm_norm_sr;
      heart_rate = BPM80;
      resp_rate = RESP12;
      enable_resp();
      switch(current_mode) {
        case 0:   // normal sinus rhythm, 80BPM
          break;
        case 1:   // normal sinus rhythm, 40BPM
          heart_rate = BPM40;
          break;
        case 2:   // normal sinus rhythm, 120BPM
          heart_rate = BPM120;
          break;
        case 3:   // normal sinus rhythm, TACH
          heart_rate = TACH;
          break;
        case 4:   // normal sinus rhythm, 80BPM, apnea
          disable_resp();
          break;
        case 5:   // normal sinus rhythm, 80BPM, hyperventillation
          resp_rate = RESP38;
          break;
        case 6:   // V-tach
          current_sequence = pwm_vtach;
          heart_rate = TACH;
          resp_rate = RESP38;
          break;
        case 7:   // V-fib
          current_sequence = pwm_vfib;
          disable_resp();
          break;
      }
    }
#endif  // ENABLE_MODE_SELECTOR
    delay(master_delay);
  }
}