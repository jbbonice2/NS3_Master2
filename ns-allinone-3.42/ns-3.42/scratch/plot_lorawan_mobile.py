#!/usr/bin/env python3
"""
Script de visualisation pour les simulations LoRaWAN mobiles
Prend en charge les versions avec et sans interférences
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import os
import sys
import argparse
from pathlib import Path

def setup_plotting():
    """Configuration des styles de graphiques"""
    plt.style.use('seaborn-v0_8')
    plt.rcParams['figure.figsize'] = (12, 8)
    plt.rcParams['font.size'] = 10
    plt.rcParams['axes.titlesize'] = 14
    plt.rcParams['axes.labelsize'] = 12
    plt.rcParams['xtick.labelsize'] = 10
    plt.rcParams['ytick.labelsize'] = 10
    plt.rcParams['legend.fontsize'] = 10
    
    # Couleurs personnalisées
    colors = ['#2E86AB', '#A23B72', '#F18F01', '#C73E1D', '#4CAF50', '#9C27B0']
    return colors

def load_csv_data(result_dir):
    """Charge les données CSV depuis le dossier de résultats"""
    csv_files = list(Path(result_dir).glob('*.csv'))
    
    if not csv_files:
        print(f"Aucun fichier CSV trouvé dans {result_dir}")
        return None
    
    # Prendre le premier fichier CSV trouvé
    csv_file = csv_files[0]
    print(f"Chargement des données depuis: {csv_file}")
    
    try:
        df = pd.read_csv(csv_file)
        print(f"Données chargées: {len(df)} enregistrements")
        print(f"Colonnes disponibles: {list(df.columns)}")
        return df
    except Exception as e:
        print(f"Erreur lors du chargement: {e}")
        return None

def create_mobility_plots(df, output_dir, colors):
    """Crée les graphiques spécifiques à la mobilité"""
    
    # 1. Trajectoires des dispositifs (échantillon)
    plt.figure(figsize=(12, 8))
    # Prendre quelques dispositifs pour éviter la surcharge
    sample_devices = df['deviceId'].unique()[:10]
    for i, device_id in enumerate(sample_devices):
        device_data = df[df['deviceId'] == device_id].sort_values('time')
        plt.plot(device_data['x'], device_data['y'], 
                alpha=0.7, marker='o', markersize=3, 
                label=f'Device {device_id}', color=colors[i % len(colors)])
    
    plt.xlabel('Position X (m)')
    plt.ylabel('Position Y (m)')
    plt.title('Trajectoires de quelques dispositifs mobiles')
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/device_trajectories.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 2. Évolution temporelle des métriques
    df['time'] = pd.to_datetime(df['time'])
    df_time = df.set_index('time').resample('10S').mean()
    
    plt.figure(figsize=(12, 8))
    plt.plot(df_time.index, df_time['success'], 
             marker='o', linewidth=2, color=colors[0])
    plt.xlabel('Temps')
    plt.ylabel('Taux de succès')
    plt.title('Évolution temporelle du taux de succès')
    plt.grid(True, alpha=0.3)
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/success_rate_over_time.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 3. RSSI vs temps pour différents SF
    plt.figure(figsize=(12, 8))
    for i, sf in enumerate(sorted(df['sf'].unique())):
        sf_data = df[df['sf'] == sf].set_index('time').resample('10S')['rssi'].mean()
        plt.plot(sf_data.index, sf_data.values, 
                marker='o', linewidth=2, label=f'SF{sf}', 
                color=colors[i % len(colors)])
    
    plt.xlabel('Temps')
    plt.ylabel('RSSI moyen (dBm)')
    plt.title('Évolution temporelle du RSSI par Spreading Factor')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/rssi_over_time_sf.png', dpi=300, bbox_inches='tight')
    plt.close()

def create_success_rate_plots(df, output_dir, colors):
    """Crée les graphiques de taux de succès"""
    
    # 1. Taux de succès par SF et TxPower
    plt.figure(figsize=(12, 8))
    success_rate = df.groupby(['sf', 'txPower'])['success'].mean().reset_index()
    pivot_success = success_rate.pivot(index='sf', columns='txPower', values='success')
    sns.heatmap(pivot_success, annot=True, fmt='.2%', cmap='RdYlGn', 
                cbar_kws={'label': 'Taux de succès'})
    plt.title('Taux de succès par Spreading Factor et Puissance de transmission')
    plt.xlabel('Puissance de transmission (dBm)')
    plt.ylabel('Spreading Factor')
    plt.tight_layout()
    plt.savefig(f'{output_dir}/success_rate_sf_txPower.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 2. Taux de succès par SF et Payload
    plt.figure(figsize=(12, 8))
    success_rate = df.groupby(['sf', 'payload'])['success'].mean().reset_index()
    pivot_success = success_rate.pivot(index='sf', columns='payload', values='success')
    sns.heatmap(pivot_success, annot=True, fmt='.2%', cmap='RdYlGn', 
                cbar_kws={'label': 'Taux de succès'})
    plt.title('Taux de succès par Spreading Factor et Taille du payload')
    plt.xlabel('Taille du payload (octets)')
    plt.ylabel('Spreading Factor')
    plt.tight_layout()
    plt.savefig(f'{output_dir}/success_rate_sf_payload.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 3. Taux de succès par SF et Bandwidth
    if 'bw' in df.columns:
        plt.figure(figsize=(12, 8))
        success_rate = df.groupby(['sf', 'bw'])['success'].mean().reset_index()
        pivot_success = success_rate.pivot(index='sf', columns='bw', values='success')
        sns.heatmap(pivot_success, annot=True, fmt='.2%', cmap='RdYlGn', 
                    cbar_kws={'label': 'Taux de succès'})
        plt.title('Taux de succès par Spreading Factor et Bande passante')
        plt.xlabel('Bande passante (Hz)')
        plt.ylabel('Spreading Factor')
        plt.tight_layout()
        plt.savefig(f'{output_dir}/success_rate_sf_bw.png', dpi=300, bbox_inches='tight')
        plt.close()

def create_metrics_plots(df, output_dir, colors):
    """Crée les graphiques de métriques par SF"""
    
    # 1. RSSI moyen par SF
    plt.figure(figsize=(12, 8))
    rssi_stats = df.groupby('sf')['rssi'].agg(['mean', 'std']).reset_index()
    plt.bar(rssi_stats['sf'], rssi_stats['mean'], 
            yerr=rssi_stats['std'], capsize=5, alpha=0.7, color=colors[0])
    plt.xlabel('Spreading Factor')
    plt.ylabel('RSSI moyen (dBm)')
    plt.title('RSSI moyen par Spreading Factor')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/rssi_vs_sf.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 2. SNR moyen par SF
    plt.figure(figsize=(12, 8))
    snr_stats = df.groupby('sf')['snr'].agg(['mean', 'std']).reset_index()
    plt.bar(snr_stats['sf'], snr_stats['mean'], 
            yerr=snr_stats['std'], capsize=5, alpha=0.7, color=colors[1])
    plt.xlabel('Spreading Factor')
    plt.ylabel('SNR moyen (dB)')
    plt.title('SNR moyen par Spreading Factor')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/snr_vs_sf.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 3. Énergie consommée par SF
    if 'energyConsumed' in df.columns:
        plt.figure(figsize=(12, 8))
        energy_stats = df.groupby('sf')['energyConsumed'].agg(['mean', 'std']).reset_index()
        plt.bar(energy_stats['sf'], energy_stats['mean'], 
                yerr=energy_stats['std'], capsize=5, alpha=0.7, color=colors[2])
        plt.xlabel('Spreading Factor')
        plt.ylabel('Énergie consommée (J)')
        plt.title('Énergie consommée par Spreading Factor')
        plt.grid(True, alpha=0.3)
        plt.tight_layout()
        plt.savefig(f'{output_dir}/energy_vs_sf.png', dpi=300, bbox_inches='tight')
        plt.close()
    
    # 4. Time on Air par SF
    if 'timeOnAir' in df.columns:
        plt.figure(figsize=(12, 8))
        toa_stats = df.groupby('sf')['timeOnAir'].agg(['mean', 'std']).reset_index()
        plt.bar(toa_stats['sf'], toa_stats['mean'], 
                yerr=toa_stats['std'], capsize=5, alpha=0.7, color=colors[3])
        plt.xlabel('Spreading Factor')
        plt.ylabel('Time on Air (ms)')
        plt.title('Time on Air par Spreading Factor')
        plt.grid(True, alpha=0.3)
        plt.tight_layout()
        plt.savefig(f'{output_dir}/toa_vs_sf.png', dpi=300, bbox_inches='tight')
        plt.close()

def create_distance_plots(df, output_dir, colors):
    """Crée les graphiques en fonction de la distance"""
    
    # 1. RSSI vs Distance par SF
    plt.figure(figsize=(12, 8))
    for i, sf in enumerate(sorted(df['sf'].unique())):
        sf_data = df[df['sf'] == sf]
        plt.scatter(sf_data['distance'], sf_data['rssi'], 
                   alpha=0.5, label=f'SF{sf}', s=10, color=colors[i % len(colors)])
    plt.xlabel('Distance (m)')
    plt.ylabel('RSSI (dBm)')
    plt.title('RSSI vs Distance par Spreading Factor')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/rssi_vs_distance_sf.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 2. SNR vs Distance par SF
    plt.figure(figsize=(12, 8))
    for i, sf in enumerate(sorted(df['sf'].unique())):
        sf_data = df[df['sf'] == sf]
        plt.scatter(sf_data['distance'], sf_data['snr'], 
                   alpha=0.5, label=f'SF{sf}', s=10, color=colors[i % len(colors)])
    plt.xlabel('Distance (m)')
    plt.ylabel('SNR (dB)')
    plt.title('SNR vs Distance par Spreading Factor')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/snr_vs_distance_sf.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 3. Évolution de la distance au cours du temps
    plt.figure(figsize=(12, 8))
    # Prendre quelques dispositifs pour éviter la surcharge
    sample_devices = df['deviceId'].unique()[:5]
    for i, device_id in enumerate(sample_devices):
        device_data = df[df['deviceId'] == device_id].sort_values('time')
        plt.plot(device_data['time'], device_data['distance'], 
                alpha=0.7, marker='o', markersize=3, 
                label=f'Device {device_id}', color=colors[i % len(colors)])
    
    plt.xlabel('Temps')
    plt.ylabel('Distance (m)')
    plt.title('Évolution de la distance au cours du temps')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/distance_over_time.png', dpi=300, bbox_inches='tight')
    plt.close()

def create_interference_plots(df, output_dir, colors):
    """Crée les graphiques d'interférence si disponibles"""
    
    if 'interferenceLoss' not in df.columns:
        print("Pas de données d'interférence disponibles")
        return
    
    # 1. Impact des interférences par SF
    plt.figure(figsize=(12, 8))
    interference_stats = df.groupby('sf')['interferenceLoss'].agg(['mean', 'std']).reset_index()
    plt.bar(interference_stats['sf'], interference_stats['mean'], 
            yerr=interference_stats['std'], capsize=5, alpha=0.7, color='red')
    plt.xlabel('Spreading Factor')
    plt.ylabel('Perte d\'interférence (dB)')
    plt.title('Impact des interférences par Spreading Factor')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/interference_impact_sf.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 2. Évolution temporelle des interférences
    df['time'] = pd.to_datetime(df['time'])
    df_time = df.set_index('time').resample('10S')['interferenceLoss'].mean()
    
    plt.figure(figsize=(12, 8))
    plt.plot(df_time.index, df_time.values, 
             marker='o', linewidth=2, color='red')
    plt.xlabel('Temps')
    plt.ylabel('Perte d\'interférence moyenne (dB)')
    plt.title('Évolution temporelle des interférences')
    plt.grid(True, alpha=0.3)
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/interference_over_time.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 3. Histogramme des pertes d'interférence
    plt.figure(figsize=(12, 8))
    plt.hist(df['interferenceLoss'], bins=50, alpha=0.7, color='red', edgecolor='black')
    plt.xlabel('Perte d\'interférence (dB)')
    plt.ylabel('Fréquence')
    plt.title('Distribution des pertes d\'interférence')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/interference_distribution.png', dpi=300, bbox_inches='tight')
    plt.close()

def create_summary_plots(df, output_dir, colors):
    """Crée des graphiques de résumé"""
    
    # 1. Statistiques générales
    plt.figure(figsize=(15, 10))
    
    # Subplot 1: Taux de succès par SF
    plt.subplot(2, 2, 1)
    success_by_sf = df.groupby('sf')['success'].mean()
    plt.bar(success_by_sf.index, success_by_sf.values, alpha=0.7, color=colors[0])
    plt.title('Taux de succès par SF')
    plt.ylabel('Taux de succès')
    plt.xlabel('Spreading Factor')
    
    # Subplot 2: RSSI moyen par SF
    plt.subplot(2, 2, 2)
    rssi_by_sf = df.groupby('sf')['rssi'].mean()
    plt.bar(rssi_by_sf.index, rssi_by_sf.values, alpha=0.7, color=colors[1])
    plt.title('RSSI moyen par SF')
    plt.ylabel('RSSI (dBm)')
    plt.xlabel('Spreading Factor')
    
    # Subplot 3: SNR moyen par SF
    plt.subplot(2, 2, 3)
    snr_by_sf = df.groupby('sf')['snr'].mean()
    plt.bar(snr_by_sf.index, snr_by_sf.values, alpha=0.7, color=colors[2])
    plt.title('SNR moyen par SF')
    plt.ylabel('SNR (dB)')
    plt.xlabel('Spreading Factor')
    
    # Subplot 4: Distribution des distances
    plt.subplot(2, 2, 4)
    plt.hist(df['distance'], bins=30, alpha=0.7, color=colors[3], edgecolor='black')
    plt.title('Distribution des distances')
    plt.ylabel('Fréquence')
    plt.xlabel('Distance (m)')
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/summary_plots.png', dpi=300, bbox_inches='tight')
    plt.close()

def generate_report(df, output_dir):
    """Génère un rapport statistique"""
    report_file = f'{output_dir}/simulation_report.txt'
    
    with open(report_file, 'w') as f:
        f.write("=== RAPPORT DE SIMULATION LORAWAN MOBILE ===\n\n")
        
        f.write(f"Nombre total de messages: {len(df)}\n")
        f.write(f"Nombre de dispositifs: {df['deviceId'].nunique()}\n")
        f.write(f"Période de simulation: {df['time'].min()} à {df['time'].max()}\n\n")
        
        f.write("=== STATISTIQUES GÉNÉRALES ===\n")
        f.write(f"Taux de succès global: {df['success'].mean():.2%}\n")
        f.write(f"RSSI moyen: {df['rssi'].mean():.2f} dBm\n")
        f.write(f"SNR moyen: {df['snr'].mean():.2f} dB\n")
        f.write(f"Distance moyenne: {df['distance'].mean():.2f} m\n")
        f.write(f"Distance min: {df['distance'].min():.2f} m\n")
        f.write(f"Distance max: {df['distance'].max():.2f} m\n")
        
        if 'energyConsumed' in df.columns:
            f.write(f"Énergie consommée moyenne: {df['energyConsumed'].mean():.6f} J\n")
        
        if 'interferenceLoss' in df.columns:
            f.write(f"Perte d'interférence moyenne: {df['interferenceLoss'].mean():.2f} dB\n")
        
        f.write("\n=== ANALYSE PAR SPREADING FACTOR ===\n")
        sf_stats = df.groupby('sf').agg({
            'success': 'mean',
            'rssi': 'mean',
            'snr': 'mean',
            'distance': 'mean'
        }).round(3)
        
        if 'energyConsumed' in df.columns:
            sf_stats['energyConsumed'] = df.groupby('sf')['energyConsumed'].mean()
        
        if 'interferenceLoss' in df.columns:
            sf_stats['interferenceLoss'] = df.groupby('sf')['interferenceLoss'].mean()
        
        f.write(sf_stats.to_string())
        f.write("\n\n")
        
        f.write("=== ANALYSE PAR PUISSANCE DE TRANSMISSION ===\n")
        power_stats = df.groupby('txPower').agg({
            'success': 'mean',
            'rssi': 'mean',
            'snr': 'mean',
            'distance': 'mean'
        }).round(3)
        
        if 'energyConsumed' in df.columns:
            power_stats['energyConsumed'] = df.groupby('txPower')['energyConsumed'].mean()
        
        f.write(power_stats.to_string())
        f.write("\n\n")
        
        f.write("=== ANALYSE PAR TAILLE DE PAYLOAD ===\n")
        payload_stats = df.groupby('payload').agg({
            'success': 'mean',
            'rssi': 'mean',
            'snr': 'mean'
        }).round(3)
        
        if 'timeOnAir' in df.columns:
            payload_stats['timeOnAir'] = df.groupby('payload')['timeOnAir'].mean()
        
        f.write(payload_stats.to_string())
        
        f.write("\n\n=== ANALYSE DE LA MOBILITÉ ===\n")
        f.write(f"Nombre de positions uniques: {df[['x', 'y']].drop_duplicates().shape[0]}\n")
        f.write(f"Variance position X: {df['x'].var():.2f}\n")
        f.write(f"Variance position Y: {df['y'].var():.2f}\n")
        
        # Calculer la vitesse moyenne si possible
        if len(df) > 1:
            device_movements = []
            for device_id in df['deviceId'].unique():
                device_data = df[df['deviceId'] == device_id].sort_values('time')
                if len(device_data) > 1:
                    distances = []
                    for i in range(1, len(device_data)):
                        prev_pos = device_data.iloc[i-1]
                        curr_pos = device_data.iloc[i]
                        dist = np.sqrt((curr_pos['x'] - prev_pos['x'])**2 + 
                                     (curr_pos['y'] - prev_pos['y'])**2)
                        distances.append(dist)
                    if distances:
                        device_movements.extend(distances)
            
            if device_movements:
                f.write(f"Mouvement moyen entre transmissions: {np.mean(device_movements):.2f} m\n")
        
    print(f"Rapport sauvegardé dans {report_file}")

def main():
    parser = argparse.ArgumentParser(description='Visualisation des résultats de simulation LoRaWAN mobile')
    parser.add_argument('result_dir', nargs='?', default='lorawan_mobile_results',
                        help='Dossier contenant les résultats de simulation')
    args = parser.parse_args()
    
    # Vérifier que le dossier existe
    if not os.path.exists(args.result_dir):
        print(f"Erreur: Le dossier {args.result_dir} n'existe pas.")
        print("Veuillez d'abord exécuter la simulation.")
        return
    
    # Charger les données
    df = load_csv_data(args.result_dir)
    if df is None:
        return
    
    # Créer le dossier de sortie
    output_dir = f"{args.result_dir}/plots"
    os.makedirs(output_dir, exist_ok=True)
    
    # Configuration des graphiques
    colors = setup_plotting()
    
    # Créer les graphiques
    print("Création des graphiques de mobilité...")
    create_mobility_plots(df, output_dir, colors)
    
    print("Création des graphiques de taux de succès...")
    create_success_rate_plots(df, output_dir, colors)
    
    print("Création des graphiques de métriques...")
    create_metrics_plots(df, output_dir, colors)
    
    print("Création des graphiques de distance...")
    create_distance_plots(df, output_dir, colors)
    
    print("Création des graphiques d'interférence...")
    create_interference_plots(df, output_dir, colors)
    
    print("Création des graphiques de résumé...")
    create_summary_plots(df, output_dir, colors)
    
    print("Génération du rapport...")
    generate_report(df, output_dir)
    
    print(f"Analyse terminée! Résultats sauvegardés dans {output_dir}")

if __name__ == "__main__":
    main()
