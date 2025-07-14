#!/usr/bin/env python3
"""
Script de visualisation pour les simulations LoRaWAN mixtes
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

def identify_mobility_type(df):
    """Identifie le type de mobilité des dispositifs"""
    mobility_info = {}
    
    for device_id in df['deviceId'].unique():
        device_data = df[df['deviceId'] == device_id]
        
        # Vérifier si les positions changent
        unique_positions = device_data[['x', 'y']].drop_duplicates().shape[0]
        
        if unique_positions == 1:
            mobility_info[device_id] = 'static'
        else:
            mobility_info[device_id] = 'mobile'
    
    df['mobility_type'] = df['deviceId'].map(mobility_info)
    return df

def create_mixed_mobility_plots(df, output_dir, colors):
    """Crée les graphiques spécifiques aux simulations mixtes"""
    
    # 1. Répartition des dispositifs par type de mobilité
    plt.figure(figsize=(12, 8))
    mobility_counts = df.groupby('deviceId')['mobility_type'].first().value_counts()
    plt.pie(mobility_counts.values, labels=mobility_counts.index, 
            autopct='%1.1f%%', colors=colors[:len(mobility_counts)])
    plt.title('Répartition des dispositifs par type de mobilité')
    plt.axis('equal')
    plt.tight_layout()
    plt.savefig(f'{output_dir}/mobility_distribution.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 2. Comparaison des performances par type de mobilité
    plt.figure(figsize=(12, 8))
    
    # Subplot 1: Taux de succès
    plt.subplot(2, 2, 1)
    success_by_mobility = df.groupby('mobility_type')['success'].mean()
    plt.bar(success_by_mobility.index, success_by_mobility.values, 
            alpha=0.7, color=colors[:len(success_by_mobility)])
    plt.title('Taux de succès par type de mobilité')
    plt.ylabel('Taux de succès')
    
    # Subplot 2: RSSI moyen
    plt.subplot(2, 2, 2)
    rssi_by_mobility = df.groupby('mobility_type')['rssi'].mean()
    plt.bar(rssi_by_mobility.index, rssi_by_mobility.values, 
            alpha=0.7, color=colors[:len(rssi_by_mobility)])
    plt.title('RSSI moyen par type de mobilité')
    plt.ylabel('RSSI (dBm)')
    
    # Subplot 3: SNR moyen
    plt.subplot(2, 2, 3)
    snr_by_mobility = df.groupby('mobility_type')['snr'].mean()
    plt.bar(snr_by_mobility.index, snr_by_mobility.values, 
            alpha=0.7, color=colors[:len(snr_by_mobility)])
    plt.title('SNR moyen par type de mobilité')
    plt.ylabel('SNR (dB)')
    
    # Subplot 4: Distance moyenne
    plt.subplot(2, 2, 4)
    distance_by_mobility = df.groupby('mobility_type')['distance'].mean()
    plt.bar(distance_by_mobility.index, distance_by_mobility.values, 
            alpha=0.7, color=colors[:len(distance_by_mobility)])
    plt.title('Distance moyenne par type de mobilité')
    plt.ylabel('Distance (m)')
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/mobility_comparison.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 3. Trajectoires des dispositifs mobiles
    plt.figure(figsize=(12, 8))
    mobile_devices = df[df['mobility_type'] == 'mobile']['deviceId'].unique()
    sample_mobile = mobile_devices[:10]  # Échantillon pour éviter la surcharge
    
    for i, device_id in enumerate(sample_mobile):
        device_data = df[df['deviceId'] == device_id].sort_values('time')
        plt.plot(device_data['x'], device_data['y'], 
                alpha=0.7, marker='o', markersize=3, 
                label=f'Device {device_id}', color=colors[i % len(colors)])
    
    # Ajouter les dispositifs statiques
    static_devices = df[df['mobility_type'] == 'static'].groupby('deviceId').first()
    plt.scatter(static_devices['x'], static_devices['y'], 
               s=100, alpha=0.7, color='red', marker='s', label='Dispositifs statiques')
    
    plt.xlabel('Position X (m)')
    plt.ylabel('Position Y (m)')
    plt.title('Trajectoires des dispositifs mobiles et positions statiques')
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/mixed_trajectories.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 4. Évolution temporelle par type de mobilité
    df['time'] = pd.to_datetime(df['time'])
    
    plt.figure(figsize=(12, 8))
    for mobility_type in df['mobility_type'].unique():
        mobility_data = df[df['mobility_type'] == mobility_type]
        time_series = mobility_data.set_index('time').resample('10S')['success'].mean()
        plt.plot(time_series.index, time_series.values, 
                marker='o', linewidth=2, label=f'{mobility_type.capitalize()}')
    
    plt.xlabel('Temps')
    plt.ylabel('Taux de succès')
    plt.title('Évolution temporelle du taux de succès par type de mobilité')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/success_rate_over_time_mobility.png', dpi=300, bbox_inches='tight')
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
    
    # 3. Taux de succès par SF et type de mobilité
    plt.figure(figsize=(12, 8))
    success_rate = df.groupby(['sf', 'mobility_type'])['success'].mean().reset_index()
    pivot_success = success_rate.pivot(index='sf', columns='mobility_type', values='success')
    sns.heatmap(pivot_success, annot=True, fmt='.2%', cmap='RdYlGn', 
                cbar_kws={'label': 'Taux de succès'})
    plt.title('Taux de succès par Spreading Factor et Type de mobilité')
    plt.xlabel('Type de mobilité')
    plt.ylabel('Spreading Factor')
    plt.tight_layout()
    plt.savefig(f'{output_dir}/success_rate_sf_mobility.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 4. Taux de succès par SF et Bandwidth
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
    
    # 3. Distance vs type de mobilité
    plt.figure(figsize=(12, 8))
    for mobility_type in df['mobility_type'].unique():
        mobility_data = df[df['mobility_type'] == mobility_type]
        plt.hist(mobility_data['distance'], bins=30, alpha=0.7, 
                label=f'{mobility_type.capitalize()}', density=True)
    
    plt.xlabel('Distance (m)')
    plt.ylabel('Densité')
    plt.title('Distribution des distances par type de mobilité')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/distance_distribution_mobility.png', dpi=300, bbox_inches='tight')
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
    
    # 2. Interférences par type de mobilité
    plt.figure(figsize=(12, 8))
    interference_by_mobility = df.groupby('mobility_type')['interferenceLoss'].agg(['mean', 'std']).reset_index()
    plt.bar(interference_by_mobility['mobility_type'], interference_by_mobility['mean'], 
            yerr=interference_by_mobility['std'], capsize=5, alpha=0.7, color='red')
    plt.xlabel('Type de mobilité')
    plt.ylabel('Perte d\'interférence moyenne (dB)')
    plt.title('Impact des interférences par type de mobilité')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/interference_impact_mobility.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    # 3. Évolution temporelle des interférences
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
    
    # Subplot 4: Répartition mobilité
    plt.subplot(2, 2, 4)
    mobility_counts = df.groupby('deviceId')['mobility_type'].first().value_counts()
    plt.pie(mobility_counts.values, labels=mobility_counts.index, 
            autopct='%1.1f%%', colors=colors[:len(mobility_counts)])
    plt.title('Répartition des dispositifs')
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/summary_plots.png', dpi=300, bbox_inches='tight')
    plt.close()

def generate_report(df, output_dir):
    """Génère un rapport statistique"""
    report_file = f'{output_dir}/simulation_report.txt'
    
    with open(report_file, 'w') as f:
        f.write("=== RAPPORT DE SIMULATION LORAWAN MIXTE ===\n\n")
        
        f.write(f"Nombre total de messages: {len(df)}\n")
        f.write(f"Nombre de dispositifs: {df['deviceId'].nunique()}\n")
        f.write(f"Période de simulation: {df['time'].min()} à {df['time'].max()}\n\n")
        
        # Statistiques de mobilité
        mobility_counts = df.groupby('deviceId')['mobility_type'].first().value_counts()
        f.write("=== RÉPARTITION DES DISPOSITIFS ===\n")
        for mobility_type, count in mobility_counts.items():
            f.write(f"{mobility_type.capitalize()}: {count} dispositifs ({count/len(mobility_counts)*100:.1f}%)\n")
        f.write("\n")
        
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
        
        f.write("\n=== ANALYSE PAR TYPE DE MOBILITÉ ===\n")
        mobility_stats = df.groupby('mobility_type').agg({
            'success': 'mean',
            'rssi': 'mean',
            'snr': 'mean',
            'distance': 'mean'
        }).round(3)
        
        if 'energyConsumed' in df.columns:
            mobility_stats['energyConsumed'] = df.groupby('mobility_type')['energyConsumed'].mean()
        
        if 'interferenceLoss' in df.columns:
            mobility_stats['interferenceLoss'] = df.groupby('mobility_type')['interferenceLoss'].mean()
        
        f.write(mobility_stats.to_string())
        f.write("\n\n")
        
        f.write("=== ANALYSE PAR SPREADING FACTOR ===\n")
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
        
    print(f"Rapport sauvegardé dans {report_file}")

def main():
    parser = argparse.ArgumentParser(description='Visualisation des résultats de simulation LoRaWAN mixte')
    parser.add_argument('result_dir', nargs='?', default='lorawan_mixed_results',
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
    
    # Identifier les types de mobilité
    df = identify_mobility_type(df)
    
    # Créer le dossier de sortie
    output_dir = f"{args.result_dir}/plots"
    os.makedirs(output_dir, exist_ok=True)
    
    # Configuration des graphiques
    colors = setup_plotting()
    
    # Créer les graphiques
    print("Création des graphiques de mobilité mixte...")
    create_mixed_mobility_plots(df, output_dir, colors)
    
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
