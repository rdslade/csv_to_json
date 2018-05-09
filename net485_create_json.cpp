#include <iostream>
#include <fstream>
#include "csvstream.h"
#include <string>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <set>

using namespace std;

vector<string> split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

string decToHex(int decimal_value, int length) {
	char hex[4];
	string format = "%0" + to_string(length) + "x";
	sprintf(hex, format.c_str(), decimal_value);
	string hexS = (string(hex));
	for (auto & c : hexS) c = toupper(c);
	return hexS;
}

bool isOneMore(string a, string b){
  int a_i = stoi(a, nullptr, 16);
  int b_i = stoi(b, nullptr, 16);
  return a_i == b_i + 1;
}

class Map{
public:
	Map(bool read, string type, string start, int num_in)
		: mrcsrc(read ? "0101" : "0102"), vartype(type), startAddr(start), num(num_in) {}
	string toJSON(int id) {
		ostringstream json;
		json << "    {" << endl;
		json << "      " << "\"id\": \"" << setw(4) << setfill('0') << decToHex(id, 4) << "\"," << endl;
		json << "      " << "\"mrcsrc\": \"" << mrcsrc << "\"," << endl;
		json << "      " << "\"vartype\": \"" << vartype << "\"," << endl;
		json << "      " << "\"start\": \"" << startAddr << "\"," << endl;
		json << "      " << "\"num\": \"" << decToHex(num, 4) << "\"" << endl;
		json << "    }";
		return json.str();
	}

private:
  string mrcsrc;
  string vartype;
  string startAddr;
  int num;
};

class DataSet{
public:
  DataSet(string name_) : name(name_), maps(vector<Map>()){}
  void print(){
    cout << name << endl;
  }
  string getName(){
    return name;
  }
  Map getMap() {
	  return maps[0];
  }
  void addMap(Map * map_in) {
	  maps.push_back(*map_in);
  }
private:
  string name;
  vector<Map> maps;
};

class Device{
public:
  Device(string name_) : name(name_), datasets(vector<DataSet*>()){}
  void print(){
    cout << name << endl;
    for(unsigned int i = 0; i < datasets.size(); i++){
      cout << "\t";
      datasets[i]->print();
    }
  }
  string getName() const{
    return name;
  }
  void addDataset(DataSet * in){
    datasets.push_back(in);
  }
  DataSet * getDataSetAt(int index) {
	  return datasets[index];
  }
  int getDataSetsSize() const {
	  return datasets.size();
  }

private:
  string name;
  vector<DataSet*> datasets;
};

class Converter{
public:
  Converter(string file_in_, string file_out_) :
	  file_in(file_in_), file_out(file_out_), csvin(file_in_){
  }


  void setDevices(){
    map<string, string> row;
    csvin >> row;
    for (auto col:row) {
      string column_name = col.first; //aka device heading
      vector<string> splitDeviceName = split(column_name, '=');
      if(splitDeviceName.size() >= 2){ //if the column has an equal sign, then it is of interest
        Device * d = new Device(splitDeviceName[1]);
        devices.push_back(d);
        setDeviceDatasets(*d);
      }
    }
  }

  void print(){
    for(unsigned int i = 0; i < devices.size(); i++){
      Device temp = *(devices[i]);
      temp.print();
    }
  }

  //void sortDevices() {
	 // for (unsigned int i = 0; i < devices.size(); ++i) {
		//  Device * temp = devices[i];
		//  //for each device, sort it's dataset array
		//  temp
	 // }
  //}

  string convertToJSON() {
	  //need to initilize data structures
	  //devices array is good enough for that b/c no repeat devices
	  //potential repeats for datasets; set up as map
	  unsigned int datasetCounter = 0;
	  using Pair_type = pair<int, Map>;
	  map<string, Pair_type> dataS;

	  for (unsigned int i = 0; i < devices.size(); ++i) {
		  Device temp = *(devices[i]);
		  for (auto j = 0; j < temp.getDataSetsSize(); ++j) {
			  DataSet * ds = temp.getDataSetAt(j);
			  if (dataS.find(ds->getName()) == dataS.end()) {
				  //we should insert this new part of the map
				  Pair_type dummy = { ++datasetCounter, ds->getMap() };
				  dataS.insert(pair<string, Pair_type>(ds->getName() , dummy));
			  }
		  }
	  }

	  ostringstream json;
	  json << "{" << endl;
	  json << "  " << "\"devs\": [" << endl;
	  for (unsigned int deviceCounter = 0; deviceCounter < devices.size(); ++deviceCounter) {
		  if (deviceCounter) {
			  json << "," << endl;
		  }
		  json << "    " << "{" << endl;
		  json << "      " << "\"id\": \"" << setw(3) << setfill('0') << decToHex(deviceCounter + 1, 3) << "\"," << endl;
		  Device * cur = devices[deviceCounter];
		  json << "      " << "\"devname\": \"" << removeFirstSpace(cur->getName()) << "\"," << endl;
		  json << "      " << "\"datasets\": [";
		  //add all ids of the device's datasets
		  for (int i = 0; i < cur->getDataSetsSize(); ++i) {
			  DataSet * tempDataSet = cur->getDataSetAt(i);
			  string id = decToHex(dataS.find(tempDataSet->getName())->second.first, 4);
			  if (i) {
				  json << ",";
			  }
			  json << endl;
			  json << "        " << "\"" << id << "\"";
		  }
		  json << endl << "      " << "]" << endl;
		  json << "    " << "}";
	  }
	  json << endl << "  ]," << endl;
	  json << "  " << "\"dsets\": [" << endl;
	  // Declaring the type of Predicate that accepts 2 pairs and return a bool
	  typedef function<bool(pair<string, Pair_type>, pair<string, Pair_type>)> Comparator;

	  // Defining a lambda function to compare two pairs. It will compare two pairs using second field
	  Comparator compFunctor =
		  [](pair<string, Pair_type> e1, pair<string, Pair_type> e2)
	  {
		  return e1.second.first < e2.second.first;
	  };

	  // Declaring a set that will store the pairs using above comparision logic
	  set<pair<string, Pair_type>, Comparator> sortedSet(
		  dataS.begin(), dataS.end(), compFunctor);
	  bool stillFirst = true;
	  for (auto it = sortedSet.begin(); it != sortedSet.end(); ++it) {
		  if (stillFirst) {
			  stillFirst = false;
		  }
		  else {
			  json << "," << endl;
		  }
		  json << "    " << "{" << endl;
		  json << "      " << "\"id\": \"" << setw(4) << setfill('0') << decToHex(it->second.first, 4) << "\"," << endl;
		  json << "      " << "\"dataname\": \"" << removeFirstSpace(it->first) << "\"," << endl;
		  json << "      " << "\"mappings\": [";
		  json << endl <<  "        \"" << setw(4) << setfill('0') << decToHex(it->second.first, 4) << "\"" << endl;
		  json << "      " << "]" << endl;
		  json << "    " << "}";

	  }
	  json << endl << "  ]," << endl;
	  json << "  " << "\"maps\": [" << endl;
	  stillFirst = true;
	  int mapCounter = 0;
	  for (auto it = sortedSet.begin(); it != sortedSet.end(); ++it) {
		  if (stillFirst) {
			  stillFirst = false;
		  }
		  else {
			  json << "," << endl;
		  }
		  Map tempMap = it->second.second;
		  json << tempMap.toJSON(++mapCounter);
	  }
	  json << endl << "  ]";
	  json << endl << "}" << endl;
	  return json.str();
  }

private:
  const string file_in;
  const string file_out;
  csvstream csvin;
  vector<Device*> devices;
//  vector<DataSet> datasets;
//  vector<Map> maps;

  string removeFirstSpace(string in) {
	if (in[0] == ' ') {
		in.erase(in.begin(), in.begin() + 1);
	}
	return in;
  }

  void setDeviceDatasets(Device &d){
    csvstream csvin2(file_in);
    map<string, string> row;
    while(csvin2 >> row){
      for(auto col : row){
        if(col.first == "Device Name =" + d.getName()){
          string datum = col.second;
          vector<string> splitDatumName = split(datum, '=');
          if(splitDatumName.size() >= 2){ //if the column has an equal sign, then it is of interest
            DataSet * data = new DataSet(splitDatumName[1]);
            d.addDataset(data);
            setMappings(csvin2, data, d);
          }
        }
      }
    }
  }

  void setMappings(csvstream & csv, DataSet * data, Device & dev){
    map<string, string> row;
    csv >> row;
    bool read = false;
    for(auto col : row){
      if(col.first == "Device Name =" + dev.getName()){
        read = (col.second.find("Read") != string::npos) ? true : false;
        break;
      }
    }
    csv >> row >> row;
    for(auto col = row.begin(); col != row.end(); ++col){
	    string deviceHeader = "Device Name =" + dev.getName();
      if(col->first == deviceHeader){
        string type, start;
        type = col->second;
		    string addressHeader = "Address" + dev.getName();
        start = row[addressHeader];
        //start of tracking this mapping
    	  int numElements = 1;
        string curAddress = start;
		    while (csv >> row && row[deviceHeader] != "") {
          string nextAddress = row[addressHeader];
          if(isOneMore(nextAddress, curAddress)){
    			     ++numElements;
          }
          else{
            //detected new mapping
            //finish setting current mapping
            data->addMap(new Map(read, type, start, numElements));
            //TODO move onto next mapping
            //TODO how to detect which elements are reads vs writes?
          }
          curAddress = nextAddress;
		    }
        data->addMap(new Map(read, type, start, numElements));
		    break;
      }
    }
	//add map to given dataset
  }
};

int main(int argc, char *argv[]){
  string file_in = string(argv[1]);
  string file_out = string(argv[2]);
  /*cout << "Enter the input CSV filename: ";
  cin >> file_in;
  cout << "Enter the output JSON filename: ";
  cin >> file_out;*/
  try {
	  Converter convert(file_in, file_out);
	  cout << "Processing input data..." << endl;
	  convert.setDevices();
	  //convert now has the structure of each device/datasets/maps
	  //need to convert to a readable json format
	  cout << endl << "Converting to JSON format..." << endl;
	  ofstream out;
	  out.open(file_out);
	  out << convert.convertToJSON() << endl;
	  out.close();
	  cout << endl << "Completed converting " << file_in << " to " << file_out << endl;
	  return 0;
  }
  catch (const csvstream_exception &e) {
	  cout << e.msg << endl;
	  return 1;
  }
}
