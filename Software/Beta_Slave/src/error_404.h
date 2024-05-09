/*******************************************************************************
 * Programmers: Jake Lewis, Zachary Harrington, Nicholas Gerth, Matthew Stavig *                                                      
 * Class: EE416 - Electrical Engineering Design                                *
 * Sponsoring Company: Philips                                                 *
 * Industry Mentor: Scott Schweizer                                            *
 * Faculty Mentor: Mohammad Torabi                                             *
 *                                                                             *
 *                          Patient Monitor Project                            *
 *                                                                             *
 * Date: 5/8/2024                                                              *
 * File: error_404.h                                                           *
 *                                                                             *
 * Description: HTML for error 404                                             *
 *                                                                             *
 *                                                                             *
 ******************************************************************************/

const char *HTML_CONTENT_404 = R""""(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
    <title>404 - Page Not Found</title>
    <style>
        h1 {color: #ff4040;}
    </style>
</head>
<body>
    <h1>404</h1>
    <p>Oops! The page you are looking for could not be found on Arduino Web Server.</p>
    <p>Please check the URL or go back to the <a href="/">homepage</a>.</p>
</body>
</html>
)"""";
