import pandas as pd
import argparse

def calculate_statistics(file_path):
    """
    Reads a CSV file and calculates basic statistics for numeric columns.
    Args:
        file_path (str): Path to the input CSV file.
    Returns:
        pd.DataFrame: A DataFrame containing statistics for each numeric column.
    """
    # Read the CSV file
    df = pd.read_csv(file_path)
    
    # Filter numeric columns
    numeric_cols = df.select_dtypes(include=['number'])
    
    # Compute statistics
    stats = {
        "mean": numeric_cols.mean(),
        "median": numeric_cols.median(),
        "std_dev": numeric_cols.std(),
        "min": numeric_cols.min(),
        "max": numeric_cols.max(),
        "25th_percentile": numeric_cols.quantile(0.25),
        "75th_percentile": numeric_cols.quantile(0.75),
    }
    
    # Convert to a DataFrame
    stats_df = pd.DataFrame(stats)
    stats_df.index.name = 'Column'
    
    return stats_df

def main():
    # Argument parser for command-line inputs
    parser = argparse.ArgumentParser(description="Calculate statistics for numeric columns in CSV files.")
    parser.add_argument("input_file1", type=str, help="Path to the first CSV file.")
    parser.add_argument("input_file2", type=str, help="Path to the second CSV file.")
    parser.add_argument("output_file", type=str, help="Path to save the statistics (CSV format).")
    
    # Parse arguments
    args = parser.parse_args()
    
    # Calculate statistics for both files
    print(f"Processing {args.input_file1}...")
    stats1 = calculate_statistics(args.input_file1)
    print(f"Processing {args.input_file2}...")
    stats2 = calculate_statistics(args.input_file2)
    
    # Combine results
    combined_stats = pd.concat({
        "File1": stats1,
        "File2": stats2
    }, axis=1)
    
    # Save to CSV
    combined_stats.to_csv(args.output_file)
    print(f"Statistics saved to {args.output_file}")

if __name__ == "__main__":
    main()
