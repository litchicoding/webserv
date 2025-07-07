#!/usr/bin/env php
<?php
header('Content-Type: text/html');
echo "<!DOCTYPE html><html><head><title>PHP Form</title></head><body>";
echo "<h1>Formulaire PHP</h1>";
echo "<form method='POST' action='process.php'>";
echo "<input type='text' name='name' placeholder='Nom' required><br><br>";
echo "<input type='text' name='email' placeholder='Email' required><br><br>";
echo "<button type='submit'>Envoyer</button>";
echo "</form>";
echo "</body></html>";
?>