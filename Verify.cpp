#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>
#include <stdexcept>

static const int EXIT_USAGE = 1;
static const int EXIT_MISSING_ARGS = 2;
static const int EXIT_META_READ_ERROR = 3;
static const int EXIT_SIG_FAILED = 4;

static void log_error(const std::string &msg) {
    std::cerr << "ERROR: " << msg << std::endl;
}

static std::map<std::string, std::string> read_meta(const std::string &filepath) {
    std::map<std::string, std::string> kv_map;
    std::ifstream meta_stream(filepath);

    if (!meta_stream.is_open()) {
        return kv_map;
    }

    std::string line;
    while (std::getline(meta_stream, line)) {
        if (line.empty()) {
            continue;
        }

        size_t delim_pos = line.find(':');
        if (delim_pos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, delim_pos);
        std::string value = line.substr(delim_pos + 1);

        if (!key.empty()) {
            kv_map[key] = value;
        }
    }

    return kv_map;
}

static void print_meta(const std::map<std::string, std::string> &metadata) {
    std::cout << "\n===== META =====\n";
    for (const auto &entry : metadata) {
        std::cout << entry.first << " : " << entry.second << "\n";
    }
}

static int run_command(const std::string &cmd) {
    return std::system(cmd.c_str());
}

static void show_usage() {
    std::cout << "\nFirmware Verification Tool\n\n";
    std::cout << "Usage:\n";
    std::cout << "  verify_tool --meta <meta.txt> --cert <cert.pem> --bin <file.bin> --sig <file.sig> --pub <public.pem>\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help        Display this help message\n";
    std::cout << "  --meta        Path to metadata file\n";
    std::cout << "  --cert        Path to X.509 certificate file\n";
    std::cout << "  --bin         Path to firmware binary\n";
    std::cout << "  --sig         Path to signature file\n";
    std::cout << "  --pub         Path to public key file (PEM format)\n\n";
}

int main(int argc, char *argv[]) {
    std::string meta_path, cert_path, bin_path, sig_path, pub_path;

    if (argc < 2) {
        show_usage();
        return EXIT_USAGE;
    }

    for (int idx = 1; idx < argc; ++idx) {
        std::string arg = argv[idx];

        if (arg == "--help") {
            show_usage();
            return 0;
        } else if (arg == "--meta" && idx + 1 < argc) {
            meta_path = argv[++idx];
        } else if (arg == "--cert" && idx + 1 < argc) {
            cert_path = argv[++idx];
        } else if (arg == "--bin" && idx + 1 < argc) {
            bin_path = argv[++idx];
        } else if (arg == "--sig" && idx + 1 < argc) {
            sig_path = argv[++idx];
        } else if (arg == "--pub" && idx + 1 < argc) {
            pub_path = argv[++idx];
        }
    }

    if (meta_path.empty() || cert_path.empty() || bin_path.empty() || sig_path.empty() || pub_path.empty()) {
        log_error("Missing required arguments");
        show_usage();
        return EXIT_MISSING_ARGS;
    }

    std::map<std::string, std::string> metadata = read_meta(meta_path);
    if (metadata.empty()) {
        log_error("Failed to read metadata file or file is empty: " + meta_path);
        return EXIT_META_READ_ERROR;
    }

    print_meta(metadata);

    std::cout << "\n===== CERT INFO =====\n";
    std::string cert_cmd = "openssl x509 -in " + cert_path + " -noout -subject -issuer -dates";
    run_command(cert_cmd);

    std::cout << "\n===== SIGNATURE CHECK =====\n";
    std::string verify_cmd = "openssl dgst -sha256 -verify " + pub_path +
                             " -signature " + sig_path + " " + bin_path;

    int verify_result = run_command(verify_cmd);
    if (verify_result != 0) {
        std::cout << "SIGNATURE: FAILED\n";
        return EXIT_SIG_FAILED;
    }

    std::cout << "SIGNATURE: OK\n";
    std::cout << "\nDONE\n";

    return 0;
    }
