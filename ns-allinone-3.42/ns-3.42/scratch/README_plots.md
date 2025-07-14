# Scripts de Visualisation LoRaWAN

Ce dossier contient les scripts Python pour analyser et visualiser les résultats des simulations LoRaWAN.

## Scripts disponibles

### 1. `plot_lorawan_mixed_interf.py`
Script principal pour analyser les résultats de simulation mixte (statique + mobile) avec interférences.

**Usage :**
```bash
# Avec dossier par défaut
python plot_lorawan_mixed_interf.py

# Avec dossier personnalisé
python plot_lorawan_mixed_interf.py chemin/vers/dossier/
```

### 2. `plot_lorawan_mobile_interf.py`
Script pour analyser spécifiquement les résultats de simulation mobile avec interférences.

**Usage :**
```bash
# Avec dossier par défaut
python plot_lorawan_mobile_interf.py

# Avec dossier personnalisé
python plot_lorawan_mobile_interf.py chemin/vers/dossier/
```

## Dépendances

```bash
pip install pandas matplotlib seaborn numpy
```

Ou utilisez le fichier requirements.txt du projet principal :
```bash
pip install -r ../requirements.txt
```

## Structure des données d'entrée

Les scripts attendent des fichiers CSV avec les colonnes suivantes :
- `deviceId` : ID du dispositif
- `messageId` : ID du message
- `time` : Timestamp au format datetime
- `x`, `y`, `z` : Position du dispositif
- `distance` : Distance à la gateway
- `txPower` : Puissance de transmission (dBm)
- `sf` : Spreading Factor
- `bw` : Bande passante (Hz)
- `cr` : Coding Rate
- `payload` : Taille du payload (octets)
- `nDevices` : Nombre total de dispositifs
- `rssi` : RSSI mesuré (dBm)
- `snr` : SNR mesuré (dB)
- `success` : Succès de la transmission (0/1)
- `energyConsumed` : Énergie consommée (mWh)
- `timeOnAir` : Temps d'émission (ms)
- `totalTx` : Nombre total de transmissions
- `totalRx` : Nombre total de réceptions
- `interferenceLoss` : Perte due aux interférences (dB)

## Graphiques générés

### Taux de succès
- Taux de succès par SF et puissance de transmission
- Taux de succès par SF et taille de payload
- Taux de succès par SF et bande passante
- Heatmaps des taux de succès

### Métriques par paramètre
- RSSI, SNR, énergie consommée, Time on Air par SF
- Comparaisons entre différents paramètres LoRa

### Analyses temporelles
- Évolution du RSSI, SNR, énergie dans le temps
- Comparaisons par SF et puissance de transmission
- Efficacité énergétique temporelle

### Analyses par nombre de dispositifs
- PDR (Packet Delivery Ratio) en fonction du nombre de dispositifs
- Métriques moyennes par nombre de dispositifs
- Efficacité énergétique par nombre de dispositifs

### Analyses par message
- Évolution des métriques par numéro de message
- Comparaisons entre combinaisons de paramètres

## Dossiers de sortie

Les graphiques sont sauvegardés dans des sous-dossiers `*_plots/` :
- `lorawan_mixed_results_interf/lorawan-logistics-mab-mixed_ALL_plots/`
- `lorawan_mobile_results_interf/lorawan-logistics-mab-mobile_interf_plots/`

## Personnalisation

### Modifier les paramètres de graphiques
Éditez directement les scripts pour :
- Changer les tailles de figures
- Modifier les couleurs et styles
- Ajuster les paramètres de visualisation

### Ajouter de nouveaux graphiques
Créez de nouvelles fonctions dans les scripts suivant le modèle :

```python
def plot_custom_metric(df, metric, groupby, fname, ylabel=None):
    plt.figure(figsize=(12, 7))
    # Votre code de visualisation ici
    plt.savefig(os.path.join(PLOTS_DIR, fname), dpi=300)
    plt.close()
```

## Exemples d'utilisation

### Analyse rapide
```bash
# Générer tous les graphiques automatiquement
python plot_lorawan_mixed_interf.py lorawan_mixed_results_interf/
```

### Analyse spécifique
```python
import pandas as pd
import matplotlib.pyplot as plt

# Charger les données
df = pd.read_csv('lorawan_mixed_results_interf/lorawan-logistics-mab-mixed_ALL.csv')

# Créer un graphique personnalisé
plt.figure(figsize=(10, 6))
df.groupby('sf')['success'].mean().plot(kind='bar')
plt.title('Taux de succès par SF')
plt.show()
```

## Dépannage

### Erreur de fichier non trouvé
```bash
# Vérifier que le fichier CSV existe
ls -la lorawan_mixed_results_interf/

# Vérifier le contenu du fichier
head lorawan_mixed_results_interf/lorawan-logistics-mab-mixed_ALL.csv
```

### Erreur de dépendances
```bash
# Installer les dépendances manquantes
pip install -r ../requirements.txt
```

### Problèmes de mémoire
Pour de gros fichiers de données, ajoutez au début du script :
```python
import matplotlib
matplotlib.use('Agg')  # Backend non-interactif
```

## Support

Pour signaler des problèmes ou suggérer des améliorations, créez une issue dans le repository du projet.
