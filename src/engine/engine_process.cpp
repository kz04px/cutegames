#include "engine_process.hpp"
#include <boost/process.hpp>
#include "engine.hpp"

[[nodiscard]] ProcessEngine::ProcessEngine(const id_type id, const std::string &path, const std::string &parameters)
    : Engine(id),
      m_child(path + (parameters.empty() ? "" : (" " + parameters)),
              boost::process::std_out > m_out,
              boost::process::std_in < m_in) {
}

[[nodiscard]] ProcessEngine::ProcessEngine(const id_type id,
                                           const std::string &path,
                                           const std::string &parameters,
                                           callback_type recv,
                                           callback_type send)
    : Engine(id, recv, send),
      m_child(path + (parameters.empty() ? "" : (" " + parameters)),
              boost::process::std_out > m_out,
              boost::process::std_in < m_in) {
}

ProcessEngine::~ProcessEngine() {
}

[[nodiscard]] auto ProcessEngine::is_running() -> bool {
    return m_child.running();
}

auto ProcessEngine::send(const std::string &msg) -> void {
    m_send(msg);
    m_in << msg << std::endl;
}

auto ProcessEngine::wait_for(const std::string &msg) -> void {
    std::string line;
    while (is_running()) {
        std::getline(m_out, line);
        m_recv(line);
        if (line == msg) {
            break;
        }
    }
}

auto ProcessEngine::wait_for(const std::function<bool(const std::string_view msg)> func) -> void {
    std::string line;
    auto exit = false;
    while (is_running() && !exit) {
        std::getline(m_out, line);
        m_recv(line);
        exit = func(line);
    }
}
