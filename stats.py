import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Load CSV files
file_path_old = 'results/car.csv'             # Old implementation
file_path_new = 'results/car_generic.csv'   # New implementation

# Read the old implementation data
data_old = pd.read_csv(file_path_old)
data_old['Profile'] = 'specialized profile'

# Read the new implementation data
data_new = pd.read_csv(file_path_new)
data_new['Profile'] = 'generic profile'

# Combine datasets
data = pd.concat([data_old, data_new], ignore_index=True)

# Remove 'thread index' column if it exists
if 'thread index' in data.columns:
    data = data.drop(columns=['thread index'])

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
    profile_stats = {
        **duration_stats,
        **visited_nodes_stats,
        'correlation_coefficient': correlation
    }

    # Save statistical data to a CSV file for each profile
    output_path = f'stats_{profile.replace(" ", "_")}.csv'
    pd.DataFrame([profile_stats]).to_csv(output_path, index=False)
    print(f"Statistical data for {profile} saved to {output_path}")

    # Store stats in the dictionary for potential future use
    stats[profile] = profile_stats

# Display the statistical data
print("\nStatistical Summary:")
for profile, profile_stats in stats.items():
    print(f"\nProfile: {profile}")
    for key, value in profile_stats.items():
        print(f"  {key}: {value}")

# Plot scatter plot of duration vs visited nodes for both profiles
plt.figure(figsize=(10, 6))
plt.scatter(
    data_old['visited nodes'],
    data_old['duration in microseconds'],
    color='red',
    label='specialized profile',
    alpha=0.5,
    edgecolor='black'
)
plt.scatter(
    data_new['visited nodes'],
    data_new['duration in microseconds'],
    color='blue',
    label='generic profile',
    alpha=0.5,
    edgecolor='black'
)
plt.xlabel('Visited Nodes')
plt.ylabel('Duration in Microseconds')
plt.title('Duration vs. Visited Nodes')
plt.legend()
plt.grid(True, linestyle='--', linewidth=0.5)
plt.show()
