#include <gtk/gtk.h>      // GTK library for GUI
#include <curl/curl.h>    // client URL library for making HTTP requests
#include "json.hpp"       // JSON library for parsing data from server to client
#include <iostream>       // For standard input/output operations
#include <sstream>        // For building URL strings
#include <string>         // For handling strings
#include <fstream>        // For file handling (saving data to file)
#include <glib.h>         // For UTF-8 conversion to avoid encoding issues

using namespace std;
using json = nlohmann::json;  // Define json namespace for easier usage

const string API_KEY = "05a8e84dce6e0b449ac37c11eade8dc4";// API Key for OpenWeatherMap
//Inheritance
class BaseWeatherAPI {  // Base class for weather API operations
protected:
    string city;             // Store the city name
    string responseData;     // Store the API response data

public:
    BaseWeatherAPI(const string& cityName) : city(cityName) {} // Constructor to initialize city
    virtual void fetchWeather() = 0; // Pure virtual function for fetching weather data
    virtual void displayWeather() = 0; // Pure virtual function for displaying data
    virtual void saveToFile() = 0; // Pure virtual function for saving data to file
    static string getAPIKey() {   // Static method to return the API key
        return API_KEY;
    }
};
// Derived class for implementing weather data functionalities
class WeatherAPI : public BaseWeatherAPI {
public:
    WeatherAPI(const string& cityName) : BaseWeatherAPI(cityName) {} // Constructor to initialize city
    void fetchWeather() override;   // Fetch weather data from API
    void displayWeather() override; // Display weather data
    void saveToFile() override;     // Save data to a file
};
GtkWidget *entry, *outputLabel; // GTK Widgets for text entry and output display

// Callback function to handle HTTP response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;  // Calculate total data size
    output->append((char*)contents, totalSize); // Append received data to string
    return totalSize;
}
string make_utf8_safe(const string& input) {   // Function to ensure text is UTF-8 safe
    GError* error = NULL;
    gchar* utf8_str = g_locale_to_utf8(input.c_str(), -1, NULL, NULL, &error);
    if (error) {                    // Handle UTF-8 conversion errors
        g_error_free(error);
        return "Invalid UTF-8 data";
    }
    string safe_string = utf8_str;
    g_free(utf8_str);                // Free allocated memory
    return safe_string;
}
void WeatherAPI::fetchWeather() {   // Fetch weather data from OpenWeatherMap API
    ostringstream urlStream;     // Create URL string stream
    urlStream << "http://api.openweathermap.org/data/2.5/weather?q=" << city
              << "&appid=" << API_KEY << "&units=metric";
    string url = urlStream.str();

    CURL* curl = curl_easy_init(); // Initialize cURL session
    if (!curl) {
        responseData = "Failed to initialize cURL."; // Error if initialization fails
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());   // Set API endpoint URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // Define response handler
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);     // Save response data
    curl_easy_setopt(curl, CURLOPT_CAINFO, "C:\\Users\\Abcom\\OneDrive\\Desktop\\Weather map SLA\\cacert.pem"); // SSL certificate

    CURLcode res = curl_easy_perform(curl); // Perform the request
    curl_easy_cleanup(curl);                // Cleanup cURL session

    if (res != CURLE_OK) {                  // Check for request failure
        responseData = "Failed to fetch weather data.";
    }
}
void WeatherAPI::displayWeather() {   // Display weather data and save details to file
    try {
        json weatherData = json::parse(responseData); // Parse JSON data
        if (weatherData.contains("message")) {        // Handle API error response
            gtk_label_set_text(GTK_LABEL(outputLabel), make_utf8_safe("API Error: " + weatherData["message"].get<string>()).c_str());
            return;
        }
        // Extract weather information
        string cityName = weatherData["name"];
        double temperature = weatherData["main"]["temp"];
        int humidity = weatherData["main"]["humidity"];
        string condition = weatherData["weather"][0]["description"];
        // Format output data
        string outputText = "City: " + cityName + "\nTemperature: " + to_string(temperature) +
                             "\u00B0C\nHumidity: " + to_string(humidity) + "%\nWeather: " + condition;

        gtk_label_set_text(GTK_LABEL(outputLabel), make_utf8_safe(outputText).c_str()); // Display data

        ofstream file("weather_data.txt", ios::app); // Save data to file
        if (file.is_open()) {
            file << "City: " << cityName << endl;
            file << "Temperature: " << temperature << "°C" << endl;
            file << "Humidity: " << humidity << "%" << endl;
            file << "Weather: " << condition << endl;
            file << "-----------------------------------" << endl;
            file.close();
        }
    } catch (const exception& e) {     // Handle JSON parsing errors
        gtk_label_set_text(GTK_LABEL(outputLabel), "Error parsing JSON.");
    }
}

// Save raw JSON data to file
void WeatherAPI::saveToFile() {
    ofstream file("raw_weather_data.json");
    if (file.is_open()) {
        file << responseData;
        file.close();
    }
}

// GTK callback to trigger data fetch
void on_fetch_weather(GtkWidget* widget, gpointer data) {
    const gchar* city = gtk_entry_get_text(GTK_ENTRY(entry)); // Get entered city name
    if (strlen(city) == 0) { // Handle empty input
        gtk_label_set_text(GTK_LABEL(outputLabel), "Please enter a city name.");
        return;
    }

    WeatherAPI weather(city);  // Create WeatherAPI object
    weather.fetchWeather();    // Fetch weather data
    weather.displayWeather();  // Display weather data
    weather.saveToFile();      // Save data to file
}

// Initialize GTK GUI elements
void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window, *grid, *button;
    window = gtk_application_window_new(app); // Create window
    gtk_window_set_title(GTK_WINDOW(window), "Weather App");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    grid = gtk_grid_new();  // Create grid layout
    gtk_container_add(GTK_CONTAINER(window), grid);
    entry = gtk_entry_new(); // Create text entry field
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter city...");
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 2, 1);
    button = gtk_button_new_with_label("Get Weather"); // Create button
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 2, 1);
    g_signal_connect(button, "clicked", G_CALLBACK(on_fetch_weather), NULL); // Connect button click event
    outputLabel = gtk_label_new("Weather data will appear here."); // Create output label
    gtk_grid_attach(GTK_GRID(grid), outputLabel, 0, 2, 2, 1);
    gtk_widget_show_all(window); // Display all widgets
}

int main(int argc, char** argv) {
    GtkApplication* app = gtk_application_new("com.example.weather", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv); // Run GTK application
    g_object_unref(app); // Free memory
    return status;       // Return application status
}
