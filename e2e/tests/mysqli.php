<?php
/*
 * Copyright 2021 SkyAPM
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
function testMysqli() {
    $mysqli = new mysqli("127.0.0.1", "root", "password", "skywalking");
    if (mysqli_connect_errno()) {
        printf("Connect failed: %s\n", mysqli_connect_error());
        exit();
    }

    mysqli_query($mysqli,"CREATE TABLE mock (name VARCHAR(25))");

    $mysqli->autocommit(FALSE);
    $sql = "INSERT INTO mock (name) VALUES (?)";
    $stmt = $mysqli->prepare($sql);
    $stmt->bind_param("s",$val1);
    $val1 = 'A';
    $stmt->execute();
    $stmt->close();
    mysqli_commit($mysqli);

    $mysqli->query("DROP TABLE mock");
    mysqli_close($mysqli);
}
