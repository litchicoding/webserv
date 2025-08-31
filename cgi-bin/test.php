<?php
header("Content-Type: text/html; charset=UTF-8");
echo "<html><body>";
echo "<h1>Données reçues en PHP</h1>";
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    echo "<p>POST: " . htmlspecialchars(file_get_contents('php://input'), ENT_QUOTES, 'UTF-8') . "</p>";
} else {
    echo "<p>GET: " . htmlspecialchars($_GET['data'] ?? '', ENT_QUOTES, 'UTF-8') . "</p>";
}
echo "</body></html>";
?>
