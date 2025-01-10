import pandas as pd
import numpy as np
import argparse

def analyze_data_updated(file_path_old, file_path_new, output_path):
    # Read the old implementation data
    data_old = pd.read_csv(file_path_old)
    data_old['Profile'] = 'specialized profile'

    # Read the new implementation data
    data_new = pd.read_csv(file_path_new)
    data_new['Profile'] = 'generic profile'

    # Combine datasets
    data = pd.concat([data_old, data_new], ignore_index=True)

    # List of profiles
    profiles = data['Profile'].unique()

    # Initialize a dictionary to hold stats for each profile
    stats = {}

    for profile in profiles:
        profile_data = data[data['Profile'] == profile]

        # Calculate statistical data for duration
        duration_stats = {
            'average_duration': profile_data['duration in microseconds'].mean(),
            'median_duration': profile_data['duration in microseconds'].median(),
            'min_duration': profile_data['duration in microseconds'].min(),
            'max_duration': profile_data['duration in microseconds'].max(),
            '25th_percentile_duration': np.percentile(profile_data['duration in microseconds'], 25),
            '75th_percentile_duration': np.percentile(profile_data['duration in microseconds'], 75)
        }

        # Calculate statistical data for visited nodes
        visited_nodes_stats = {
            'average_visited_nodes': profile_data['visited nodes'].mean(),
            'median_visited_nodes': profile_data['visited nodes'].median(),
            'min_visited_nodes': profile_data['visited nodes'].min(),
            'max_visited_nodes': profile_data['visited nodes'].max(),
            '25th_percentile_visited_nodes': np.percentile(profile_data['visited nodes'], 25),
            '75th_percentile_visited_nodes': np.percentile(profile_data['visited nodes'], 75)
        }

        # Calculate correlation between visited nodes and duration
        correlation = profile_data['duration in microseconds'].corr(profile_data['visited nodes'])

        # Combine all stats into one dictionary
        stats[profile] = {
            'average_duration': duration_stats['average_duration'],
            'median_duration': duration_stats['median_duration'],
            'min_duration': duration_stats['min_duration'],
            'max_duration': duration_stats['max_duration'],
            '25th_percentile_duration': duration_stats['25th_percentile_duration'],
            '75th_percentile_duration': duration_stats['75th_percentile_duration'],
            'average_visited_nodes': visited_nodes_stats['average_visited_nodes'],
            'median_visited_nodes': visited_nodes_stats['median_visited_nodes'],
            'min_visited_nodes': visited_nodes_stats['min_visited_nodes'],
            'max_visited_nodes': visited_nodes_stats['max_visited_nodes'],
            '25th_percentile_visited_nodes': visited_nodes_stats['25th_percentile_visited_nodes'],
            '75th_percentile_visited_nodes': visited_nodes_stats['75th_percentile_visited_nodes'],
            'correlation_coefficient': correlation
        }

    # Convert stats to DataFrame
    df_stats = pd.DataFrame(stats)

    # Add percent difference column for each statistical measure
    percent_diff = (
                           abs(df_stats['specialized profile'] - df_stats['generic profile'])
                           / df_stats.mean(axis=1)
                   ) * 100

    df_stats['percent_difference'] = percent_diff

    # Save the DataFrame to CSV
    df_stats.to_csv(output_path, index_label='Statistical Measure')
    print(f"Statistical results saved to {output_path}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Analyze and compare routing profile data.")
    parser.add_argument("file_path_old", help="Path to the old implementation data CSV file.")
    parser.add_argument("file_path_new", help="Path to the new implementation data CSV file.")
    parser.add_argument("output_path", help="Path to save the output CSV file.")

    args = parser.parse_args()
    analyze_data_updated(args.file_path_old, args.file_path_new, args.output_path)
