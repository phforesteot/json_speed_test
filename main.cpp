#include <chrono>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <vector>

using namespace std;
int nrof_passes = 20;

inline void print_hhmmss_time_section(std::ostream& out, const unsigned int value)
{
    if (value == 0) {
        out << "00";
    } else if (value < 10) {
        out << "0" << value;
    } else {
        out << value;
    }
}

const static unsigned int SECONDS_IN_AN_HOUR = 3600;
const static unsigned int SECONDS_IN_A_MINUTE = 60;

inline void print_hhmmss_time(std::ostream& out, const unsigned int value)
{
    print_hhmmss_time_section(out, (unsigned int)(value / SECONDS_IN_AN_HOUR));
    out << ":";
    print_hhmmss_time_section(out, (unsigned int)((value % SECONDS_IN_AN_HOUR) / SECONDS_IN_A_MINUTE));
    out << ":";
    print_hhmmss_time_section(out, (unsigned int)((value % SECONDS_IN_AN_HOUR) % (SECONDS_IN_A_MINUTE)));
}

struct do_not_start {
};

struct stop_watch {

    std::chrono::system_clock::time_point begin_process;
    typedef double type;
    typedef std::chrono::duration<type> time_type;
    // typedef std::chrono::duration<std::chrono::milliseconds> time_type;

    stop_watch(do_not_start val)
        : begin_process()
    {
    }

    stop_watch()
        : begin_process(std::chrono::system_clock::now())
    {
    }

    void start()
    {
        begin_process = std::chrono::system_clock::now();
    }
    type seconds()
    {
        std::chrono::system_clock::time_point end_process = std::chrono::system_clock::now();

        time_type time_span = std::chrono::duration_cast<time_type>(end_process - begin_process);
        return time_span.count();
    }

    type milliseconds()
    {
        std::chrono::system_clock::time_point end_process = std::chrono::system_clock::now();

        std::chrono::milliseconds time_span = std::chrono::duration_cast<std::chrono::milliseconds>(end_process - begin_process);
        return time_span.count();
    }

    type nanosconds()
    {
        std::chrono::system_clock::time_point end_process = std::chrono::system_clock::now();

        std::chrono::nanoseconds time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end_process - begin_process);
        return time_span.count();
    }

    void stop(std::string const message = "", std::ostream& out = std::cout)
    {
        std::chrono::system_clock::time_point end_process = std::chrono::system_clock::now();
        time_type time_span = std::chrono::duration_cast<time_type>(end_process - begin_process);
        out << message;
        print_hhmmss_time(out, time_span.count());
        // std::cout << std::endl;
    }
};

nlohmann::json testnlohmann_msgapack(std::vector<std::uint8_t>& data)
{
    stop_watch sw;
    nlohmann::json jo;
    std::uint8_t const* udata = reinterpret_cast<std::uint8_t const*>(&data[0]);
    for (int i = 0; i < nrof_passes; ++i) {
        jo = nlohmann::json::from_msgpack(udata, udata + (data.size()), false);
    }
    cout << "msgpack size:" << data.size() << endl;
    cout << "msgapack:" << sw.nanosconds() << " (ns)" << endl;
    sw.stop("msgapack:");
    cout << endl;
    return jo;
}

nlohmann::json testnlohmann_bson(std::vector<std::uint8_t>& data)
{
    stop_watch sw;
    nlohmann::json jo;
    std::uint8_t const* udata = reinterpret_cast<std::uint8_t const*>(&data[0]);
    for (int i = 0; i < nrof_passes; ++i) {
        jo = nlohmann::json::from_bson(udata, udata + (data.size()), false);
    }
    cout << "bson size:" << data.size() << endl;
    cout << "bson:" << sw.nanosconds() << " (ns)" << endl;
    sw.stop("bson:");
    cout << endl;
    return jo;
}

nlohmann::json testnlohmann_cbor(std::vector<std::uint8_t>& data)
{
    stop_watch sw;
    nlohmann::json jo;
    std::uint8_t const* udata = reinterpret_cast<std::uint8_t const*>(&data[0]);
    for (int i = 0; i < nrof_passes; ++i) {
        jo = nlohmann::json::from_cbor(udata, udata + (data.size()), false);
    }
    std::cout << "cbor size:" << data.size() << endl;
    cout << "cbor:" << sw.nanosconds() << " (ns)" << endl;
    sw.stop("cbor:");
    cout << endl;
    return jo;
}

nlohmann::json testnlohmann_json(std::vector<std::uint8_t>& data)
{
    stop_watch sw;
    nlohmann::json jo;
    char const* udata = reinterpret_cast<char const*>(&data[0]);
    for (int i = 0; i < nrof_passes; ++i) {
        jo = nlohmann::json::parse(udata, udata + (data.size()));
    }
    std::cout << "json size:" << data.size() << endl;
    cout << "json:" << sw.nanosconds() << " (ns)" << endl;
    sw.stop("json:");
    cout << endl;
    return jo;
}

std::vector<unsigned char> load_binary(const char* filename)
{
    try {
        std::ifstream ifile(filename, std::ios_base::binary);
        ifile.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

        ifile.seekg(0, std::ios_base::end);
        size_t buffersize = ifile.tellg();
        std::vector<unsigned char> buffer(buffersize);
        ifile.seekg(0, std::ios_base::beg);
        ifile.read((char*)&buffer[0], buffersize);
        return buffer;
    } catch (std::exception const& ex) {
        std::stringstream error;
        error << "can not open file " << filename;
        throw std::runtime_error(error.str().c_str());
    }

    return std::vector<unsigned char>();
}

int main(int argc, char* argv[])
{

    std::vector<std::uint8_t> v_json = load_binary("test.json");
    std::vector<std::uint8_t> v_msgpack = load_binary("test.msgpack");
    std::vector<std::uint8_t> v_cbor = load_binary("test.cbor");
    std::vector<std::uint8_t> v_bson = load_binary("test.bson");

    std::cout << "start speed test" << std::endl;
    auto r4 = testnlohmann_json(v_json);
    auto r1 = testnlohmann_msgapack(v_msgpack);
    auto r3 = testnlohmann_cbor(v_cbor);
    auto r2 = testnlohmann_bson(v_bson);
}
