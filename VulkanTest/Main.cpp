#include "HelloTriangleApplication.h"


int main() {
	HelloTriangleApplication app;

	try {
		app.Run();
	} catch (const std::runtime_error& e) {
		printf("%s\n", e.what());

#ifndef NDEBUG
		system("pause");
#endif

		return EXIT_FAILURE;
	}

#ifndef NDEBUG
	system("pause");
#endif

	return EXIT_SUCCESS;
}
