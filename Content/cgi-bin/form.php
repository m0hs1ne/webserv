<!DOCTYPE html>
<html>
<head>
    <title>PHP Form Example</title>
</head>
<body>
    <h1>PHP Form Example</h1>
    <form method="post" action="form.php">
        <label for="name">Name:</label>
        <input type="text" name="name" id="name"><br><br>
        <label for="email">Email:</label>
        <input type="text" name="email" id="email"><br><br>
        <input type="submit" name="submit" value="Submit">
    </form>
    <form action="form.php" method="post" enctype="multipart/form-data">
        <input type="file" name="file" multiple>
        <input type="submit" value="Upload">
    </form>
    <?php
        if ($_SERVER["REQUEST_METHOD"] == "POST") {


            $name = $_POST["name"];
            $email = $_POST["email"];

            echo "<h2>Your Input:</h2>";
            echo "Name: " . $name . "<br>";
            echo "Email: " . $email;


        }

        if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_FILES['file'])) {
            $file = $_FILES['file'];
            if ($file['error'] === UPLOAD_ERR_OK) {
                $filename = basename($file['name']);
                $upload_dir = '/Users/mabenchi/Desktop/webserv/Content/cgi-bin/tmp/';
                $upload_path = $upload_dir . $filename;
                if (move_uploaded_file($file['tmp_name'], $upload_path)) {
                    echo "<br>File uploaded successfully.\n";
                    echo "<img src='/cgi-bin/tmp/" . $filename . "'/>";
                } else {
                    echo "<br>Error uploading file.\n";
                }
            } else {
                echo "<br>Error: " . $file['error'] . "\n";
            }
        }
    ?>

</body>
</html>