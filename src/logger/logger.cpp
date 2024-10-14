#include "logger/logger.h"

Logger* Logger::logger;

void Logger::set_active(bool isActive) {
	active = isActive;
}

bool Logger::is_active() const {
	return active;
}

Logger* Logger::get_logger() {
	if (!logger) {
		logger = new Logger();
	}
	return logger;
}

void Logger::print(std::string message) const {
	if (!active)
		return;

	std::cout << message << std::endl;
}

void Logger::shutdown() {
	if (logger) {
		delete logger;
	}
}

// Extension and Layer reporting

void Logger::print_layers(const char* layerCategory, std::vector<VkLayerProperties>& layers) const {
	if (!active)
		return;

	std::cout << layerCategory << std::endl;
	for (const auto& layer : layers) {
		std::cout << "\t" << layer.layerName << std::endl;
	}
}
void Logger::print_layers(const char* layerCategory, std::vector<const char*>& layers) const {
	if (!active)
		return;

	std::cout << layerCategory << std::endl;
	print_list(layers);
}

void Logger::print_extensions(const char* extensionCategory, std::vector<VkExtensionProperties>& extensions) const {
	if (!active)
		return;

	std::cout << extensionCategory << std::endl;
	for (const auto& extension : extensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}
}
void Logger::print_extensions(const char* extensionCategory, std::vector<const char*>& extensions) const {
	if (!active)
		return;

	std::cout << extensionCategory << std::endl;
	print_list(extensions);
}

void Logger::print_list(std::vector<const char*>& list) const {
	if (!active)
		return;

	for (const auto& member : list) {
		std::cout << "\t" << member << std::endl;
	}
}

// Version reporting

void Logger::report_version(uint32_t version) const {
	if (!active)
		return;

	std::cout << "Engine using Vulkan Variant: " << VK_API_VERSION_VARIANT(version)
		<< ", Major: " << VK_API_VERSION_MAJOR(version)
		<< ", Minor: " << VK_API_VERSION_MINOR(version)
		<< ", PATCH: " << VK_API_VERSION_PATCH(version) << std::endl;
}

void Logger::log(struct QueueFamilyIndices& indices) const {
	if (!active)
		return;

	std::cout << "There are " << indices.queueFamilyProperties.size() << " queue families in the GPU." << std::endl;
	for (uint32_t i = 0; i < indices.queueFamilyProperties.size(); i++) {
		auto family = indices.queueFamilyProperties[i];
		std::cout << "Queue Family (" << i << "):" << std::endl;
		std::cout << "\tSupports " << family.queueFlags << std::endl;
		std::cout << "\tHas " << family.queueCount << " queues" << std::endl;
	}

	std::cout << "Chosen queues to utilize:" << std::endl;
	if (indices.graphicsFamily.has_value())
		std::cout << "\tGraphics Queue (" << indices.graphicsFamily.value() << ")" << std::endl;

	if (indices.presentFamily.has_value())
		std::cout << "\t Present Queue (" << indices.presentFamily.value() << ")" << std::endl;
 }

void Logger::log(VkPhysicalDeviceProperties& physdevice) const {
	if (!active)
		return;

	std::cout << "Device name: " << physdevice.deviceName << std::endl;
	std::cout << "\tDevice type: ";
	switch (physdevice.deviceType) {
	case(VK_PHYSICAL_DEVICE_TYPE_CPU):
		std::cout << "CPU";
		break;
	case(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU):
		std::cout << "Discrete GPU";
		break;
	case(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU):
		std::cout << "Integrated GPU";
		break;
	case(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU):
		std::cout << "Virtual GPU";
		break;
	default:
		std::cout << "Other";
	}
	std::cout << std::endl;
}

void Logger::print_devices(std::vector<VkPhysicalDevice>& devices) const {
	if (!active)
		return;

	std::cout << "List of physical devices: " << std::endl;
	for (const auto& device : devices) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);
		log(properties);
	}
}