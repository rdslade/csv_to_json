# csv_to_json
Convert specific format of .csv to readable and usable JSON for Net 485 device configuration

## Installation
### Using git clone
```
$ git clone https://github.com/rdslade/csv_to_json
$ mv input_file.csv csv_to_join
$ cd csv_to_json
```
### Using downloads
1. Download csv_to_json.zip
2. Unzip or open the file in the desired location
3. Move your desired input file into the csv_to_json directory

## How to use
```
$ make json.exe 
```
```
$ ./json.exe input_file.csv output_file.json
```

## Requirements for .csv file
#### Device name declaration
  - must be in top row of document
  - must contain phrase "Device Name = device_name"
#### Dataset declaration
  - follows directly underneath device name declaration
  - must contain phrase "= dataset_name"
#### Address column in top row of document must be labeled "Address device_name"
#### Must have at least one blank line between datasets
#### Different groupings must have unique names

