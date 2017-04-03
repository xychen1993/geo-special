/*
Usage: This file first reads in a CSV file (default path is current directory and default name is Partition6467LinkData.csv) which contains all the linkdata, and then reads in a CSV file (default path is current directory and default name is Partition6467ProbePoints.csv) which contains all the probe data, then output a txt file which contanins the match information regarding probe point and link.
*/

#include <cmath>
#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <map>
using namespace std;

#define RADIAN_EARTH 6371000

void split(const string&s, const char* delim, vector<string> &v){
	char *dup = strdup(s.c_str());
	char *token = strtok(dup, delim);
	while(token != NULL){
		v.push_back(string(token));
		token = strtok(NULL, delim);
	}
	free(dup);
}

//This class represent a Geopoint on map
class Point{
public:
	float lat;
	float lon;
	float alt;
	string Id;

	Point(){}

	Point(string s){
		Id = s;
		vector<string> v;
		split(s,"/",v);
		lat = stof(v[0]);
		lon = stof(v[1]);
	}
};

//This class reprensent a link on map
class Link{
public:
	Point ref_node;
	Point non_node;
	string Id;
	//vector representation for latitude
	float vector_lat;
	//vector reprensentation for longtitude
	float vector_lon;
	//length of the link
	float length;
	//radian of the link
	float radian;

	//Construction function
	Link(){}

	Link(string inputId){
		Id = inputId;
		ref_node = Point();
		non_node = Point();
	}

	Link(string inputId, Point startp, Point endp){
		Id = inputId;
		ref_node = startp;
		non_node = endp;
		vector_lat = non_node.lat - ref_node.lat;
		vector_lon = non_node.lon - ref_node.lat;
		length = sqrt(pow(vector_lat, 2) + pow(vector_lon, 2));
		if(vector_lat != 0)
			radian = atan(vector_lon/vector_lat);
		else if(vector_lon > 0)
			radian = M_PI/2;
		else
			radian = M_PI*3/2;
	}

	//Calculate the vertial distance from the point to link
	float cal_dis_to_link(Point point){
		float diff_lat = point.lat - ref_node.lat;
		float diff_lon = point.lon - ref_node.lon;
		float dis_to_ref = pow(diff_lon, 2) + pow(diff_lat, 2);

		//calculate the project distance
		float porj_dis = (diff_lat * vector_lat + diff_lon * vector_lon) / length;
		if(porj_dis < 0)
			return dis_to_ref;
		if(porj_dis > length)
			return pow(point.lon - non_node.lon, 2) + pow(point.lat - non_node.lat, 2);

		return (pow(diff_lon, 2) + pow(diff_lat, 2) - pow(porj_dis, 2));
	}

	//Calculate the distance from point to reference point
	float cal_dis_to_ref(Point point){
		float diff_lon = point.lon - ref_node.lon;
		float diff_lat = point.lat - ref_node.lat;
		float dis_to_ref = pow(diff_lon, 2) + pow(diff_lat, 2);
		return sqrt(dis_to_ref);
	}
};

int main(){
	//Load the link data
	ifstream link_file;
	link_file.open("Partition6467LinkData.csv");

	map<string, vector<Link> > links;
	map<string, vector<Link> > points;

	if(link_file.is_open()){
		string line;
		//Parse the input
		while(getline(link_file, line)){
			vector<string> attr;
			split(line, ",", attr);
			vector<string> nodes;
			split(attr[14], "|", nodes);
			for(int i = 0; i < nodes.size() - 1; ++i){
				Link temp(attr[0], Point(nodes[i]), Point(nodes[i+1]));
				if(!links.count(attr[0]))
					links[attr[0]] = vector<Link>();
				links[attr[0]].push_back(temp);
				if(!points.count(nodes[i]))
					points[nodes[i]] = vector<Link>();
				points[nodes[i]].push_back(temp);
				if(!points.count(nodes[i+1]))
					points[nodes[i+1]] = vector<Link>();
				points[nodes[i+1]].push_back(temp);
			}
		}
	}
	cout << "Link data loaded" << endl;

	ofstream mid_output;
	mid_output.open("mapped(partial).txt");

	string last_node;
	Point potential_ref;
	Point potential_non;

	ifstream probe_file;
	probe_file.open("Partition6467ProbePoints.csv");

	if(probe_file.is_open()){
		string line;
		while(getline(probe_file, line)){
			//parse the data into variable attr
			vector<string> attr;
			split(line, ",", attr);
			string id = attr[3] + "/" + attr[4];
			Point ptr(id);
			//Initilize the smalleset distance
			float smallest_dis = -10000;
			//The link that holds the smallest distance
			string link_id;
			float dis_to_ref;
			string direction;
			//Usually one probeID has several entries, so we dont
			//constantly loop through the links to find link, we
			//only do it when the probeID changes
			if(attr[0] != last_node){
				last_node = attr[0];
				for(auto link : links){
					for(auto link_node : link.second){
						float proj_dis = link_node.cal_dis_to_link(ptr);
						//Update the smallest distance when needed
						if(smallest_dis == -10000 || proj_dis < smallest_dis){
							smallest_dis = proj_dis;
							link_id = link_node.Id;
							dis_to_ref = link.second[0].cal_dis_to_ref(ptr);
							float const_a = stof(attr[7]);
							float const_b = link_node.radian;
							if(cos(const_a) * cos(const_b) + sin(const_a) * sin(const_b) > 0)
								direction = "F";
							else
								direction = "T";
							potential_ref = link_node.ref_node;
							potential_non = link_node.non_node;
						}
					}
				}
				//
			}
			else{
				for(auto link : points[potential_ref.Id]){
					float dis = link.cal_dis_to_link(ptr);
					//Update the smallest distance if needed
					if(smallest_dis == -10000 || dis < smallest_dis){
						smallest_dis = dis;
						link_id = link.Id;
						dis_to_ref = links[link_id][0].cal_dis_to_ref(ptr);
						float const_a = stof(attr[7]);
						float const_b = link.radian;
						if(cos(const_a) * cos(const_b) + sin(const_a) * sin(const_b) > 0)
								direction = "F";
							else
								direction = "T";
					}
				}
			}
			//Using the radius of Earch
			float temp_a = sqrt(smallest_dis) * M_PI / 180 * RADIAN_EARTH;
			float temp_b = dis_to_ref * M_PI / 180 * RADIAN_EARTH;
			mid_output << attr[0] <<", " << attr[1] <<", " << attr[2] <<", " << attr[3] <<", " << attr[4] <<", " << attr[5] <<", " << attr[6] <<", " << attr[7] <<", " << link_id <<", " << direction <<", " << to_string(temp_a) <<", " << to_string(temp_b) <<"\n";
		}
	}
}