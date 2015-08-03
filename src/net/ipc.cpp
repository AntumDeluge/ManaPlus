/*
 *  The ManaPlus Client
 *  Copyright (C) 2014-2015  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "net/ipc.h"

#include "gui/windows/chatwindow.h"

#include "utils/delete2.h"
#include "utils/sdlhelper.h"
#include "utils/stringutils.h"

#include "debug.h"

IPC *ipc = nullptr;

IPC::IPC() :
    mNumReqs(0),
    mSocket(nullptr),
    mDelayedCommands(),
    mThread(nullptr),
    mMutex(SDL_CreateMutex()),
    mPort(44007U),
    mThreadLocked(false),
    mListen(false)
{
}

IPC::~IPC()
{
    mListen = false;
    if (mSocket)
    {
        TcpNet::closeSocket(mSocket);
        mSocket = nullptr;
    }
    SDL_DestroyMutex(mMutex);
    mMutex = nullptr;
    int status;
    if (mThread && SDL_GetThreadID(mThread))
        SDL_WaitThread(mThread, &status);
    mThread = nullptr;
}

bool IPC::init()
{
    IPaddress ip;

    if (TcpNet::resolveHost(&ip, nullptr, mPort) == -1)
    {
        logger->log("IPC: resolveHost error: %s\n", TcpNet::getError());
        return false;
    }

    mSocket = TcpNet::open(&ip);
    if (!mSocket)
    {
        logger->log("IPC: open error: %s", TcpNet::getError());
        return false;
    }

    mThread = SDL::createThread(&acceptLoop, "ipc", this);
    if (!mThread)
    {
        logger->log("IPC: unable to create acceptLoop thread");
        return false;
    }
    return true;
}

int IPC::acceptLoop(void *ptr)
{
    if (!ptr)
        return 1;

    IPC *const ipc1 = reinterpret_cast<IPC*>(ptr);
    const int max_length = 1024;
    const TcpNet::SocketSet set = TcpNet::allocSocketSet(1);
    TcpNet::addSocket(set, ipc1->mSocket);
    ipc->mListen = true;
    while (ipc1->mListen)
    {
        TcpNet::checkSockets(set, 250);
        if (!TcpNet::socketReady(ipc1->mSocket))
            continue;

        const TcpNet::Socket sock = TcpNet::accept(ipc1->mSocket);
        if (!sock)
        {
            logger->log_r("IPC: unable to accept connection");
            continue;
        }
        char data[max_length] = {0};
        int result = TcpNet::recv(sock, data, max_length);
        if (result <= 0)
        {
            logger->log_r("IPC: unable to accept connection");
            TcpNet::closeSocket(sock);
            continue;
        }

        std::string req(data);
        trim(req);

        ipc1->mThreadLocked = true;
        SDL_mutexP(ipc1->mMutex);
        ipc1->mDelayedCommands.push_back(req);
        SDL_mutexV(ipc1->mMutex);
        ipc1->mThreadLocked = false;

        ipc1->mNumReqs ++;
        const std::string resp = strprintf("[%u] %s\n",
            ipc1->mNumReqs, req.c_str());

        const char *const respc = resp.c_str();
        const int len = static_cast<int>(strlen(respc)) + 1;
        result = TcpNet::send(sock, respc, len);
        if (result < len)
        {
            logger->log_r("IPC: send error: %s\n", TcpNet::getError());
            TcpNet::closeSocket(sock);
            continue;
        }
        TcpNet::closeSocket(sock);
    }
    TcpNet::closeSocket(ipc1->mSocket);
    ipc1->mSocket = nullptr;
    ipc->mThread = nullptr;
    return 0;
}

void IPC::stop()
{
    if (!ipc)
        return;

    logger->log("Stopping IPC...");
    delete2(ipc);
}

void IPC::start()
{
    if (ipc)
        return;

    unsigned short port(44007);
    if (getenv("IPC_PORT"))
        port = static_cast<unsigned short>(atoi(getenv("IPC_PORT")));

    logger->log("Starting IPC...");
    ipc = new IPC;
    for (int f = port; f < 65535; f ++)
    {
        ipc->setPort(static_cast<unsigned short>(f));
        logger->log("  -> trying port %d...", f);
        if (ipc->init())
        {
            logger->log("  -> Port %d selected", f);
            return;
        }
        else
        {
            port ++;
        }
    }
    delete2(ipc);
}

void IPC::flush()
{
    if (!mThreadLocked)
    {
        SDL_mutexP(mMutex);
#ifndef DYECMD
// probably need enable only commands in tool
        if (chatWindow)
        {
            FOR_EACH (std::vector<std::string>::const_iterator, it,
                      mDelayedCommands)
            {
                chatWindow->chatInput(*it);
            }
        }
#endif
        mDelayedCommands.clear();
        SDL_mutexV(mMutex);
    }
}
