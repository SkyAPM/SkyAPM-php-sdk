<?php

function testMysqli() {
    $mysqli = new mysqli("127.0.0.1", "root", "111111", "mock");
    if (mysqli_connect_errno()) {
        printf("Connect failed: %s\n", mysqli_connect_error());
        exit();
    }

    mysqli_query($mysqli,"CREATE TABLE mock2 Like mock");

    $mysqli->autocommit(FALSE);
    $sql = "INSERT INTO mock2 (name) VALUES (?)";
    $stmt = $mysqli->prepare($sql);
    $stmt->bind_param("s",$val1);
    $val1 = 'A';
    $stmt->execute();
    $stmt->close();
    mysqli_commit($mysqli);

    $mysqli->query("DROP TABLE mock2");
    mysqli_close($mysqli);
}
