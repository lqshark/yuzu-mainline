// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <tuple>
#include "common/assert.h"
#include "core/hle/kernel/client_port.h"
#include "core/hle/kernel/errors.h"
#include "core/hle/kernel/object.h"
#include "core/hle/kernel/server_port.h"
#include "core/hle/kernel/server_session.h"
#include "core/hle/kernel/thread.h"

namespace Kernel {

ServerPort::ServerPort(KernelCore& kernel) : WaitObject{kernel} {}
ServerPort::~ServerPort() = default;

ResultVal<std::shared_ptr<ServerSession>> ServerPort::Accept() {
    if (pending_sessions.empty()) {
        return ERR_NOT_FOUND;
    }

    auto session = std::move(pending_sessions.back());
    pending_sessions.pop_back();
    return MakeResult(std::move(session));
}

void ServerPort::AppendPendingSession(std::shared_ptr<ServerSession> pending_session) {
    pending_sessions.push_back(std::move(pending_session));
}

bool ServerPort::ShouldWait(const Thread* thread) const {
    // If there are no pending sessions, we wait until a new one is added.
    return pending_sessions.empty();
}

void ServerPort::Acquire(Thread* thread) {
    ASSERT_MSG(!ShouldWait(thread), "object unavailable!");
}

ServerPort::PortPair ServerPort::CreatePortPair(KernelCore& kernel, u32 max_sessions,
                                                std::string name) {
    std::shared_ptr<ServerPort> server_port = std::make_shared<ServerPort>(kernel);
    std::shared_ptr<ClientPort> client_port = std::make_shared<ClientPort>(kernel);

    server_port->name = name + "_Server";
    client_port->name = name + "_Client";
    client_port->server_port = server_port;
    client_port->max_sessions = max_sessions;
    client_port->active_sessions = 0;

    return std::make_pair(std::move(server_port), std::move(client_port));
}

} // namespace Kernel
