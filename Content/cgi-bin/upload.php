<!DOCTYPE html>
<html>
<head>
    <title>PHP Form Example</title>
</head>
<body>
    <h1>PHP Form Example</h1>
    <form method="post" action="upload.php">
        <label for="name">Name:</label>
        <input type="text" name="name" id="name"><br><br>
        <label for="email">Email:</label>
        <input type="text" name="email" id="email"><br><br>
        <input type="file" name="fileToUpload" id="fileToUpload">
        <input type="submit" name="submit" value="Submit">
    </form>
    <?php
        $str_string = file_get_contents('php://input');
        parse_str($str_string, $_POST);
        if ($_SERVER["REQUEST_METHOD"] == "POST") {
            $name = $_POST["name"];
            $email = $_POST["email"];

            echo "<h2>Your Input:</h2>";
            echo "Name: " . $name . "<br>";
            echo "Email: " . $email;
        }
    ?>

</body>
</html>