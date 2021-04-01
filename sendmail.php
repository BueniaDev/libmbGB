<!DOCTYPE html>
<html lang="en">
    <head>
        <!--Ian Buer - WEBDEV-114 - Assignment 5 - 10/26/2020-->
        <meta http-equiv="X-UA Compatible" content="IE=edge">
        <meta name="description" content="Buer's Assignment 5">
        <meta name="keywords" content="HTML, Blog">
        <meta http-equiv="content-type" content="charset=UTF-8">
        <meta name="og:description" content="libmbbGB is an open-source Game Boy and Game Boy Color emulator, copyright &amp;copy; 2018&amp;ndash;2020 Ian Buer. This project is not affiliated in any way with Nintendo. Game Boy and Game Boy Color are registered trademarks of Nintendo Co., Ltd.">
        <title>libmbGB</title>
        <link rel="icon" href="favicon.ico" type="image/x-icon">
        <link href="https://fonts.googleapis.com/css2?family=Bebas+Neue&display=swap" rel="stylesheet">
        <link href="https://fonts.googleapis.com/css2?family=Open+Sans&display=swap" rel="stylesheet">
        <link href="https://fonts.googleapis.com/css2?family=Lato:ital,wght@1,900&display=swap" rel="stylesheet">
        <link rel="stylesheet" href="styles.css">
    </head>
    <body>
        <header id="site-header">
        <nav class="site-nav">
            <ul>
                <li><a class="active" href="contact.html">Contact</a></li>
                <li><a href="donate.html">Donate</a></li>
                <li><a href="https://github.com/Buenia0/libmbGB">Github</a></li>
                <li><a href="faq.html">FAQ</a></li>
                <li><a href="downloads.html">Downloads</a></li>
                <li><a href="index.html">Home</a></li>
            </ul>
        </nav>
        </header>
        
        <nav id="sub-nav">
            <h2>Pages</h2>
            <ul>
                <li><a href="donate.html">Donate</a></li>
                <li><a href="downloads.html">Downloads</a></li>
                <li><a href="faq.html">FAQs</a></li>
                <li><a href="contact.html">Contact</a></li>
            </ul>
            
            <h2>Links</h2>
            <ul id="nav-social">
                <li><a href="https://twitter.com/Libmbg">Twitter</a></li>
                <li><a href="https://github.com/Buenia0/libmbGB/issues">Issue Tracker</a></li>
            </ul>
        </nav>
        
        <div class="page-content">
            <div class="post">
                <header class="post-header">
                    <h1>Email Confirmation</h1>
                </header>
                <fieldset>
                    <legend>Contact Information</legend>
                    <label for="first_name">First Name:</label>
                    <input type="text" name="first_name" id="first_name" value="<?php echo $_REQUEST['first_name'] ?>" disabled><br>
                    <label for="last_name">Last Name:</label>
                    <input type="text" name="last_name" id="last_name" value="<?php echo $_REQUEST['last_name'] ?>" disabled><br>
                    <label for="email">Email Address:</label>
                    <input type="email" name="email" id="email" value="<?php echo $_REQUEST['email'] ?>" disabled><br>
                    <label for="verify">Verify Email:</label>
                    <input type="email" name="verify" id="verify" value="<?php echo $_REQUEST['email'] ?>" disabled><br>
                    <label for="phone">Phone Number:</label>
                    <input type="tel" name="phone" id="phone" value="<?php echo $_REQUEST['phone'] ?>" disabled><br>
                </fieldset>
                <fieldset>
                    <legend>Message Information</legend>
                    <label for="subject">Subject:</label>
                    <input type="text" name="subject" id="subject" value="<?php echo $_REQUEST['subject'] ?>" disabled><br>
                    <label for="Message">Message:</label>
                    <textarea id="message" name="message" rows="4" disabled><?php echo $_REQUEST['message'] ?></textarea>
                </fieldset>
                <h2>
                    <?php if (isset($_REQUEST['email'])) { //if "email" variable is filled out, send email
  
                    //Set admin email for email to be sent to (note: will be my personal address)
                    $admin_email = "buenia.mbemu@gmail.com"; 

                    //Set PHP variable equal to information completed on the HTML form
                    $email = $_REQUEST['email']; //Request email that user typed on HTML form
                    $phone = $_REQUEST['phone']; //Request phone that user typed on HTML form
                    $subject = $_REQUEST['subject']; //Request subject that user typed on HTML form
                    $message = $_REQUEST['message']; //Request message that user typed on HTML form
                    //Combine first name and last name, adding a space in between
                    $name = $_REQUEST['first_name'] . " " .  $_REQUEST['last_name']; 
            
                    //Start building the email body combining multiple values from HTML form    
                    $body  = "From: " . $name . "\n"; 
                    $body .= "Email: " . $email . "\n"; //Continue the email body
                    $body .= "Phone: " . $phone . "\n"; //Continue the email body
                    $body .= "Message: " . $message; //Continue the email body
    
                    //Create the email headers for the from and CC fields of the email     
                    $headers = "From: " . $name . " <" . $email . "> \r\n"; //Create email "from"
                    $headers .= "CC: " . $name . " <" . $email . ">"; //Send CC to visitor
    
                    //Actually send the email from the web server using the PHP mail function
                    mail($admin_email, $subject, $body, $headers); 
    
                    //Display email confirmation response on the screen
                    echo "Thank you for contacting us!"; 
                }
                //if "email" variable is not filled out, display an error
                else  { 
                    echo "There has been an error!";
                } ?>
                </h2>
            </div>
        </div>
        
        <footer id="site-footer">
            <div id="description">
                <p class="text">libmbGB is an open-source Game Boy and Game Boy Color emulator, copyright &copy; 2018&ndash;2020 Buenia. This project is not affiliated in any way with Nintendo. Game Boy and Game Boy Color are registered trademarks of Nintendo Co., Ltd.</p>
            </div>
        </footer>
    </body>
</html>