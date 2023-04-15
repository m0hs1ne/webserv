<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body>
    <form action="/upload.php" method="POST">
    <input type="text" name="name" id="name">
    <input type="submit" name="submit" id="submit">
    </form>
    <?php
        echo $_POST["name"];
    ?>
</body>
</html>