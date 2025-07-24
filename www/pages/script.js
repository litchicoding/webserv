console.log('Serveur HTTP - Test JavaScript');

function testAjax() {
    fetch('/test-endpoint')
        .then(response => response.text())
        .then(data => console.log('Réponse:', data))
        .catch(error => console.error('Erreur:', error));
}

// Test automatique au chargement
document.addEventListener('DOMContentLoaded', function() {
    console.log('Page chargée, JavaScript fonctionne !');
});