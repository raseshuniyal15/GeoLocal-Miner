#include <iostream>
#include <cstdlib>

int main() {
    std::cout << "======================================================\n"
              << "      GEOLOCAL-MINER FULL SYSTEM VERIFICATION        \n"
              << "======================================================\n" << std::endl;

    int res = 0;
    std::cout << ">>> Running Unit Test 1: Normalization..." << std::endl;
    res = std::system("bin\\test_normalization.exe");
    if (res != 0) return res;

    std::cout << ">>> Running Unit Test 2: Scoring Engine..." << std::endl;
    res = std::system("bin\\test_scoring.exe");
    if (res != 0) return res;

    std::cout << ">>> Running Unit Test 3: Priority Ranking..." << std::endl;
    res = std::system("bin\\test_ranking.exe");
    if (res != 0) return res;

    std::cout << ">>> Running Unit Test 4: Data Structures..." << std::endl;
    res = std::system("bin\\test_data_structures.exe");
    if (res != 0) return res;

    std::cout << ">>> Running Unit Test 5: Validation Engine..." << std::endl;
    res = std::system("bin\\test_validation.exe");
    if (res != 0) return res;

    std::cout << ">>> Running Unit Test 6: Data Loader & CSV I/O..." << std::endl;
    res = std::system("bin\\test_loader.exe");
    if (res != 0) return res;

    std::cout << "======================================================\n"
              << "    CONGRATULATIONS: ALL 6 TEST SUITES PASSED!       \n"
              << "======================================================\n" << std::endl;
    return 0;
}
