//============================================================================
// Name        : read_xml.cpp
// Author      : Claus
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Exception.hpp>

#include <pugixml-1.9/pugixml.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;


int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <username> <password>" << endl;
        return EXIT_FAILURE;
    }

    cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
    const string readings_url = "https://www.vegvesen.no/ws/no/vegvesen/veg/trafikkpublikasjon/vaer/2/GetMeasuredWeatherData";
    const string locations_url = "https://www.vegvesen.no/ws/no/vegvesen/veg/trafikkpublikasjon/vaer/2/GetMeasurementWeatherSiteTable";

    std::string username = argv[1];
    std::string password = argv[2];
    std::string credentials = username + ":" + password;

    map<string, map<string, string>> readings_map;
    string id, key, value, post_url = "";
    long seconds = 0;
    std::tm t = {};

    pugi::xml_document url_doc;

    curlpp::Cleanup cleaner;
    curlpp::Easy request;

    request.setOpt(new curlpp::options::Url(readings_url));
    request.setOpt(new curlpp::options::UserPwd(credentials));

    ostringstream out;
    out << request;

    pugi::xml_parse_result result = url_doc.load(out.str().c_str());

    if (result) {
        cout << "XML-weather-data parsed without errors" << endl;
    } else {
        cout << "XML-weather-data parsed with errors" << endl;
    }

	pugi::xml_node payloadPublication = url_doc.
			child("d2LogicalModel").
			child("payloadPublication");

	string child_name = "";
	for (pugi::xml_node siteMeasurement = payloadPublication.child("siteMeasurements"); siteMeasurement; siteMeasurement = siteMeasurement.next_sibling("siteMeasurements")) {
		id = siteMeasurement.child("measurementSiteReference").attribute("id").value();
//		cout << siteMeasurement.child("measurementSiteReference").attribute("id").name() << ": " << id << endl;

		key = siteMeasurement.child("measurementSiteReference").attribute("version").name();
		value = siteMeasurement.child("measurementSiteReference").attribute("version").value();
//		cout << key << ": " << value << endl;
		readings_map[id][key] = value;

		key = siteMeasurement.child("measurementTimeDefault").name();
		value =siteMeasurement.child("measurementTimeDefault").child_value();
//		cout << key << ": " << value << endl;
		readings_map[id][key] = value;

		for (pugi::xml_node measuredValue: siteMeasurement.children("measuredValue")) {
			child_name = measuredValue.child("measuredValue").first_child().first_child().first_child().name();
			// A level deeper
			if (child_name == "roadSurfaceConditionMeasurementsExtension") {
				key = measuredValue.child("measuredValue").first_child().first_child().first_child().first_child().first_child().name();
				value = measuredValue.child("measuredValue").first_child().first_child().first_child().first_child().first_child().first_child().child_value();
//				cout << key << ": " << value << endl;
				readings_map[id][key] = value;
			} else {
				key = measuredValue.child("measuredValue").first_child().first_child().first_child().name();
				value = measuredValue.child("measuredValue").first_child().first_child().first_child().first_child().child_value();
//				cout << key << ": " << value << endl;
				readings_map[id][key] = value;
			}
		}
//		cout << "---" << endl;
	}

	request.setOpt(new curlpp::options::Url(locations_url));
    request.setOpt(new curlpp::options::UserPwd(credentials));

    ostringstream out2;
    out2 << request;

    result = url_doc.load(out2.str().c_str());

    if (result) {
        cout << "XML-location-data parsed without errors" << endl;
    } else {
        cout << "XML-location-data parsed with errors" << endl;
    }

//cout << endl << out2.str() << endl;

    pugi::xml_node measurementSiteTable = url_doc.
			child("d2LogicalModel").
			child("payloadPublication").
			child("measurementSiteTable");

	for (pugi::xml_node measurementSiteRecord: measurementSiteTable.children("measurementSiteRecord")) {
		id = measurementSiteRecord.attribute("id").value();
//		cout << measurementSiteRecord.attribute("id").name() << ": " << id << endl;

		key = measurementSiteRecord.attribute("version").name();
		value = measurementSiteRecord.attribute("version").value();
//		cout << key << ": " << value << endl;
		readings_map[id][key] = value;

		key = measurementSiteRecord.child("measurementSiteName").name();
		value = measurementSiteRecord.child("measurementSiteName").child("values").child("value").child_value();
//		cout << key << ": " << value << endl;
		readings_map[id][key] = value;

		key = measurementSiteRecord.child("measurementSiteLocation").child("pointByCoordinates").child("pointCoordinates").child("latitude").name();
		value = measurementSiteRecord.child("measurementSiteLocation").child("pointByCoordinates").child("pointCoordinates").child("latitude").child_value();
//		cout << key << ": " << value << endl;
		readings_map[id][key] = value;

		key = measurementSiteRecord.child("measurementSiteLocation").child("pointByCoordinates").child("pointCoordinates").child("longitude").name();
		value = measurementSiteRecord.child("measurementSiteLocation").child("pointByCoordinates").child("pointCoordinates").child("longitude").child_value();
//		cout << key << ": " << value << endl;
		readings_map[id][key] = value;

//		cout << "----" << endl;
	}

	for (auto outer : readings_map) {
		list<string> header;
		header.push_back("Content-Type: application/json");
		request.setOpt(new curlpp::options::HttpHeader(header));
		id = outer.first;
		json j;
		j["id"] = id;
		for (auto inner : outer.second) {
//			cout << outer.first << ": " << inner.first << ": " << inner.second << endl;
			j[inner.first] = inner.second;
			if (inner.first == "measurementTimeDefault") {
				istringstream ss(inner.second);
				ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%SZ");
				seconds = std::mktime(&t);
			}
		}
		post_url = "http://localhost:9200/vegvesen_" + id + "/_doc/" + to_string(seconds); // TESTING
		request.setOpt(new curlpp::options::Url(post_url));
		request.setOpt(new curlpp::options::PostFields(j.dump()));
		request.setOpt(new curlpp::options::PostFieldSize(j.dump().length()));
		request.perform();
//		cout << j.dump() << endl;
	}

	cout << endl << "Done reading data" << endl;

	return 0;
}
