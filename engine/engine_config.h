#pragma once


struct ENG_API WindowProperties
{
	int32_t size_x, size_y;
	bool fullscreen;
};

struct ENG_API EngineProperties
{
	bool vsync;
	glm::vec3 clear_color;
};

struct ENG_API Properties
{
	WindowProperties window_properties;
	EngineProperties engine_properties;
};

class ENG_API IConfigurationParser{
public:
	virtual ~IConfigurationParser() = default;
	virtual const Eng::Properties* parse() = 0;
	virtual void setFilePath(const std::string& path) = 0;
};


class ENG_API YamlConfigurationParser: IConfigurationParser
{
private:
	std::string m_filePath;

public:
	YamlConfigurationParser() = default;
	YamlConfigurationParser(std::string filepath);

	void setFilePath(const std::string& path) override;
	const Eng::Properties* parse() override;
};

class ENG_API EngineConfigurer
{
public:
	const Eng::Properties* getConfiguration() const;
};