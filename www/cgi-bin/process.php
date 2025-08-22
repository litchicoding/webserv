#!/usr/bin/env php
<?php
header('Content-Type: text/html');
echo "<!DOCTYPE html><html><head><title>Résultat Formulaire</title></head><body>";
echo "<h1>Données reçues</h1>";
echo "Nom : " . htmlspecialchars($_POST['name']) . "<br>";
echo "Email : " . htmlspecialchars($_POST['email']) . "<br>";
echo "</body></html>";
?>
