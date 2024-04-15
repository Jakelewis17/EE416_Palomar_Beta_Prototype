/*******************************************************************************
 * Programmers: Jake Lewis, Zachary Harrington, Nicholas Gerth, Matthew Stavig *                                                      
 * Class: EE415 - Product Design Management                                    *
 * Sponsoring Company: Philips                                                 *
 * Industry Mentor: Scott Schweizer                                            *
 * Faculty Mentor: Mohammad Torabi Konjin                                      *
 *                                                                             *
 *                          Patient Monitor Project                            *
 *                                                                             *
 * Date: 4/15/2024                                                             *
 * File: index.h                                                               *
 *                                                                             *
 * Description: A patient monitor measuring the three most important           *
 *              physilogical parameters: blood oxygen, ECG, and blood pressure *   
 *                                                                             *
 *                                                                             *
 ******************************************************************************/

const char *HTML_CONTENT_HOME = R""""(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Patient Vital Monitoring</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f4f4f4;
        }

        header {
            background-color: #4D4D4D;
            color: #fff;
            padding: 10px;
            text-align: center;
        }

        .img-wsu {
            width: 8%;
            height: 12%;
            position: absolute;
            top: 10px;
            left: 16px;
        }

        .img-palomar {
            width: 8%;
            height: 12%;
            position: absolute;
            top: 10px;
            right: 16px;
        }

        
        main {
            padding: 20px;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }

        th, td {
            border: 1px solid #ddd;
            padding: 12px;
            text-align: left;
        }

        th {
            background-color: #A60F2D;
            color: white;
        }

        tr:hover {
            background-color: #f5f5f5;
        }

        footer {
            background-color: #4D4D4D;
            color: #fff;
            padding: 10px;
            text-align: center;
            position: fixed;
            bottom: 0;
            width: 100%;
        }

        a:link {
        text-decoration: none;
        }

        a:visited {
        text-decoration: none;
        }

        a:hover {
        text-decoration: underline;
        }

        a:active {
        text-decoration: underline;
        }
    </style>
</head>
<body>
    <header>
        <h1>Home Page Palomar Patient Vital Monitoring</h1>
        <img src="https://onedrive.live.com/embed?resid=29799F4508C24092%2187530&authkey=%21AGD9zoKMtX38Jz4&width=500&height=500" class="img-wsu">
        <img src="https://onedrive.live.com/embed?resid=29799F4508C24092%2187529&authkey=%21AEcU-IpMK3nJLDQ&width=290&height=233" class="img-palomar">
    </header>

    <main>
        <h2>Patient Information</h2>
        <ul>
          <li><a href="./subject1.html">Subject1 Page</a></li>
  
      </ul>

    </main>

    <footer>
        <p>&copy; 2024 Team Palomar</p>
    </footer>
</body>
</html>
)"""";
