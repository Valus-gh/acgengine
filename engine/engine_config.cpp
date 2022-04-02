#include "engine.h"
#include <fstream>
#include <sstream>


#define RYML_SINGLE_HDR_DEFINE_NOW
#include "engine_config.h"

#include "ryml_all.hpp"



#define LOG_MISSING_PROP_ERR(x) ENG_LOG_ERROR((std::string{"Error field in configuration file: "} + x).c_str())

ENG_API Eng::YamlConfigurationParser::YamlConfigurationParser(std::string filepath): m_filePath(std::move(filepath))
{
}

ENG_API void Eng::YamlConfigurationParser::setFilePath(const std::string& path)
{
	m_filePath = path;
}

ENG_API const Eng::Properties* Eng::YamlConfigurationParser::parse()
{
	std::ifstream file_stream;
	std::stringstream contents;
	file_stream.open(m_filePath);
	contents << file_stream.rdbuf();
	ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(contents.str().c_str()));

	auto* config = new Properties{};

	auto window = tree["window"];
	window["start-size"]["x"] >> config->window_properties.size_x;
	window["start-size"]["y"] >> config->window_properties.size_y;
	window["fullscreen"] >> config->window_properties.fullscreen;

	float r,g,b;

	auto engine = tree["engine"];
	engine["v-sync"] >> config->engine_properties.vsync;
	engine["clear-color"][0] >> r;
	engine["clear-color"][1] >> g;
	engine["clear-color"][2] >> b;
	config->engine_properties.clear_color = glm::vec3{r,g,b};
	
	return config;
}

ENG_API const Eng::Properties* Eng::EngineConfigurer::getConfiguration() const
{

	std::string home{ getenv("HOMEDRIVE") };
	home.append(getenv("HOMEPATH"));
	std::string configPath;

	std::ifstream f(home + "/.acg-config.yml");
	if(f.good())
	{
		ENG_LOG_INFO("Found configuration file in home directory");
		configPath = home + "/.acg-config.yml";
	}
	else
	{
		ENG_LOG_INFO("Using bundled default config file");
		configPath = "./acg-config.yml";
	}

	if(f.is_open()) f.close();

	YamlConfigurationParser parser(configPath);
	return parser.parse();
}


