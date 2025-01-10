import pandas as pd
import re
import argparse

def parse_valgrind_data(input_file):
    """
    Parses a Valgrind massif snapshot text file and returns a pandas DataFrame.
    Args:
        input_file (str): Path to the massif text file.
    Returns:
        pandas.DataFrame: DataFrame with structured data.
    """
    # Open and read the file
    with open(input_file, 'r') as file:
        lines = file.readlines()
    
    # Prepare a list for valid rows
    data_lines = []
    
    # Regex to match lines starting with a number (ignoring leading spaces)
    pattern = re.compile(r'^\s*\d+')
    
    # Extract valid lines
    for line in lines:
        if pattern.match(line):
            # Remove extra spaces, replace commas with spaces, and split into columns
            cleaned_line = re.sub(r'\s+', ' ', line.strip())
            cleaned_line = cleaned_line.replace(',', '')  # Remove any commas in the input
            data_lines.append(cleaned_line.split())
    
    # Column headers inferred from Valgrind's output structure
    columns = ["n", "time(i)", "total(B)", "useful-heap(B)", "extra-heap(B)", "stacks(B)"]
    
    # Convert data to a DataFrame
    df = pd.DataFrame(data_lines, columns=columns)
    
    # Convert numeric columns to appropriate data types
    for col in columns:
        df[col] = pd.to_numeric(df[col])
    
    return df

def main():
    # Argument parser for command-line inputs
    parser = argparse.ArgumentParser(description="Parse Valgrind massif output into structured data.")
    parser.add_argument("input_file", type=str, help="Path to the Valgrind massif output file.")
    parser.add_argument("output_file", type=str, help="Path to save the parsed data (CSV format).")
    
    # Parse the arguments
    args = parser.parse_args()
    
    # Process the file
    df = parse_valgrind_data(args.input_file)
    
    # Save to CSV
    df.to_csv(args.output_file, index=False)
    print(f"Parsed data saved to {args.output_file}")

if __name__ == "__main__":
    main()
