#include "HelloTriangleApplication.h"


int main() {
	HelloTriangleApplication app;

	try {
		app.Run();
	} catch (const std::runtime_error& e) {
		printf("%s\n", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
