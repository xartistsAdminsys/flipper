<?php
// Définir le fichier de stockage des données
$filename = 'received_data.txt';

// Vérifier si les données sont envoyées en POST
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    // Récupérer le contenu brut de la requête POST
    $data = file_get_contents('php://input');
    
    // Ouvrir le fichier pour écrire les données reçues
    $file = fopen($filename, 'a');
    if ($file) {
        fwrite($file, $data . "\n");
        fclose($file);
        echo 'Données reçues et stockées.';
    } else {
        echo 'Erreur d\'ouverture du fichier.';
    }
} else {
    echo 'Méthode de requête non supportée.';
}
?>
