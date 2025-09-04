<?php
header("Content-Type: text/html; charset=UTF-8");
echo "<html><body>";
echo "<h1>Données reçues en PHP (strict)</h1>";

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Pas de changement pour POST (on affiche toujours le body brut)
    echo "<p>POST: " . htmlspecialchars(file_get_contents('php://input'), ENT_QUOTES, 'UTF-8') . "</p>";
} else {
    // Vérification stricte pour GET
    if (count($_GET) !== 1 || !isset($_GET['data'])) {
        header("Status: 400 Bad Request");
        echo "<h2>400 Bad Request</h2>";
        echo "<p>Requête invalide : paramètre 'data' obligatoire et unique.</p>";
    } else {
        echo "<p>GET: " . htmlspecialchars($_GET['data'], ENT_QUOTES, 'UTF-8') . "</p>";
    }
}

echo "</body></html>";
?>
