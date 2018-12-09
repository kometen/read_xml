//============================================================================
// Name        : read_xml.cpp
// Author      : Claus
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <sstream>
#include <string>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Exception.hpp>

#include <pugixml-1.9/pugixml.hpp>

using namespace std;

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

    pugi::xml_document url_doc;

    curlpp::Cleanup cleaner;
    curlpp::Easy request;

    request.setOpt(new curlpp::options::Url(readings_url));
    request.setOpt(new curlpp::options::UserPwd(credentials));

    ostringstream out;
    out << request;

    pugi::xml_parse_result result = url_doc.load(out.str().c_str());

    if (result) {
        cout << "XML parsed without errors" << endl;
    } else {
        cout << "XML parsed with errors" << endl;
    }

	pugi::xml_node payloadPublication = url_doc.
			child("d2LogicalModel").
			child("payloadPublication");

	string child_name = "";
	for (pugi::xml_node siteMeasurement = payloadPublication.child("siteMeasurements"); siteMeasurement; siteMeasurement = siteMeasurement.next_sibling("siteMeasurements")) {
		cout << siteMeasurement.child("measurementSiteReference").attribute("id").name() << ": ";
		cout << siteMeasurement.child("measurementSiteReference").attribute("id").value() << endl;
		cout << siteMeasurement.child("measurementSiteReference").attribute("version").name() << ": " ;
		cout << siteMeasurement.child("measurementSiteReference").attribute("version").value() << endl;
		cout << siteMeasurement.child("measurementTimeDefault").name() << ": ";
		cout << siteMeasurement.child("measurementTimeDefault").child_value() << endl;
		for (pugi::xml_node measuredValue: siteMeasurement.children("measuredValue")) {
			child_name = measuredValue.child("measuredValue").first_child().first_child().first_child().name();
			// A level deeper
			if (child_name == "roadSurfaceConditionMeasurementsExtension") {
				cout << measuredValue.child("measuredValue").first_child().first_child().first_child().first_child().first_child().name() << ": ";
				cout << measuredValue.child("measuredValue").first_child().first_child().first_child().first_child().first_child().first_child().child_value() << endl;
			} else {
				cout << measuredValue.child("measuredValue").first_child().first_child().first_child().name() << ": ";
				cout << measuredValue.child("measuredValue").first_child().first_child().first_child().first_child().child_value() << endl;
			}
		}
		cout << "---" << endl;
	}

	request.setOpt(new curlpp::options::Url(locations_url));
    request.setOpt(new curlpp::options::UserPwd(credentials));

    ostringstream out2;
    out2 << request;

    result = url_doc.load(out2.str().c_str());

    if (result) {
        cout << "XML parsed without errors" << endl;
    } else {
        cout << "XML parsed with errors" << endl;
    }

//cout << endl << out2.str() << endl;

    pugi::xml_node measurementSiteTable = url_doc.
			child("d2LogicalModel").
			child("payloadPublication").
			child("measurementSiteTable");

	cout << measurementSiteTable.attribute("version").name() << ":: ";
	cout << measurementSiteTable.attribute("version").value() << endl;

	for (pugi::xml_node measurementSiteRecord: measurementSiteTable.children("measurementSiteRecord")) {
		cout << measurementSiteRecord.attribute("id").name() << ": ";
		cout << measurementSiteRecord.attribute("id").value() << endl;
		cout << measurementSiteRecord.attribute("version").name() << ": " ;
		cout << measurementSiteRecord.attribute("version").value() << endl;
		cout << measurementSiteRecord.child("measurementSiteName").name() << ": ";
		cout << measurementSiteRecord.child("measurementSiteName").child("values").child("value").child_value() << endl;
		cout << measurementSiteRecord.child("measurementSiteLocation").child("pointByCoordinates").child("pointCoordinates").child("latitude").name() << ": ";
		cout << measurementSiteRecord.child("measurementSiteLocation").child("pointByCoordinates").child("pointCoordinates").child("latitude").child_value() << endl;
		cout << measurementSiteRecord.child("measurementSiteLocation").child("pointByCoordinates").child("pointCoordinates").child("longitude").name() << ": ";
		cout << measurementSiteRecord.child("measurementSiteLocation").child("pointByCoordinates").child("pointCoordinates").child("longitude").child_value() << endl;
		cout << "----" << endl;
	}

	return 0;
}
