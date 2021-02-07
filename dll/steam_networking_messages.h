/* Copyright (C) 2019 Mr Goldberg
   This file is part of the Goldberg Emulator

   The Goldberg Emulator is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The Goldberg Emulator is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Goldberg Emulator; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "base.h"

class Steam_Networking_Messages :
public ISteamNetworkingMessages
{
    class Settings *settings;
    class Networking *network;
    class SteamCallResults *callback_results;
    class SteamCallBacks *callbacks;
    class RunEveryRunCB *run_every_runcb;

public:
static void steam_callback(void *object, Common_Message *msg)
{
    PRINT_DEBUG("steam_networking_messages_callback\n");

    Steam_Networking_Messages *steam_networking_messages = (Steam_Networking_Messages *)object;
    steam_networking_messages->Callback(msg);
}

static void steam_run_every_runcb(void *object)
{
    PRINT_DEBUG("steam_networking_messages_run_every_runcb\n");

    Steam_Networking_Messages *steam_networking_messages = (Steam_Networking_Messages *)object;
    steam_networking_messages->RunCallbacks();
}

Steam_Networking_Messages(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->run_every_runcb = run_every_runcb;
    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Networking_Messages::steam_callback, this);
    this->run_every_runcb->add(&Steam_Networking_Messages::steam_run_every_runcb, this);

    this->callback_results = callback_results;
    this->callbacks = callbacks;
}

~Steam_Networking_Messages()
{
    //TODO rm network callbacks
    this->run_every_runcb->remove(&Steam_Networking_Messages::steam_run_every_runcb, this);
}

/// Sends a message to the specified host.  If we don't already have a session with that user,
/// a session is implicitly created.  There might be some handshaking that needs to happen
/// before we can actually begin sending message data.  If this handshaking fails and we can't
/// get through, an error will be posted via the callback SteamNetworkingMessagesSessionFailed_t.
/// There is no notification when the operation succeeds.  (You should have the peer send a reply
/// for this purpose.)
///
/// Sending a message to a host will also implicitly accept any incoming connection from that host.
///
/// nSendFlags is a bitmask of k_nSteamNetworkingSend_xxx options
///
/// nRemoteChannel is a routing number you can use to help route message to different systems.
/// You'll have to call ReceiveMessagesOnChannel() with the same channel number in order to retrieve
/// the data on the other end.
///
/// Using different channels to talk to the same user will still use the same underlying
/// connection, saving on resources.  If you don't need this feature, use 0.
/// Otherwise, small integers are the most efficient.
///
/// It is guaranteed that reliable messages to the same host on the same channel
/// will be be received by the remote host (if they are received at all) exactly once,
/// and in the same order that they were send.
///
/// NO other order guarantees exist!  In particular, unreliable messages may be dropped,
/// received out of order with respect to each other and with respect to reliable data,
/// or may be received multiple times.  Messages on different channels are *not* guaranteed
/// to be received in the order they were sent.
///
/// A note for those familiar with TCP/IP ports, or converting an existing codebase that
/// opened multiple sockets:  You might notice that there is only one channel, and with
/// TCP/IP each endpoint has a port number.  You can think of the channel number as the
/// *destination* port.  If you need each message to also include a "source port" (so the
/// recipient can route the reply), then just put that in your message.  That is essentially
/// how UDP works!
///
/// Returns:
/// - k_EREsultOK on success.
/// - k_EResultNoConnection will be returned if the session has failed or was closed by the peer,
///   and k_nSteamNetworkingSend_AutoRestartBrokwnSession is not used.  (You can use
///   GetSessionConnectionInfo to get the details.)  In order to acknowledge the broken session
///   and start a new one, you must call CloseSessionWithUser
/// - See SendMessageToConnection::SendMessageToConnection for more
EResult SendMessageToUser( const SteamNetworkingIdentity &identityRemote, const void *pubData, uint32 cubData, int nSendFlags, int nRemoteChannel )
{
    PRINT_DEBUG("Steam_Networking_Messages::SendMessageToUser\n");
    return k_EResultNoConnection;
}

/// Reads the next message that has been sent from another user via SendMessageToUser() on the given channel.
/// Returns number of messages returned into your list.  (0 if no message are available on that channel.)
///
/// When you're done with the message object(s), make sure and call Release()!
int ReceiveMessagesOnChannel( int nLocalChannel, SteamNetworkingMessage_t **ppOutMessages, int nMaxMessages )
{
    PRINT_DEBUG("Steam_Networking_Messages::ReceiveMessagesOnChannel\n");
    return 0;
}

/// AcceptSessionWithUser() should only be called in response to a SteamP2PSessionRequest_t callback
/// SteamP2PSessionRequest_t will be posted if another user tries to send you a message, and you haven't
/// tried to talk to them.  If you don't want to talk to them, just ignore the request.
/// If the user continues to send you messages, SteamP2PSessionRequest_t callbacks will continue to
/// be posted periodically.  This may be called multiple times for a single user.
///
/// Calling SendMessage() on the other user, this implicitly accepts any pending session request.
bool AcceptSessionWithUser( const SteamNetworkingIdentity &identityRemote )
{
    PRINT_DEBUG("Steam_Networking_Messages::AcceptSessionWithUser\n");
    return false;
}

/// Call this when you're done talking to a user to immediately free up resources under-the-hood.
/// If the remote user tries to send data to you again, another P2PSessionRequest_t callback will
/// be posted.
///
/// Note that sessions that go unused for a few minutes are automatically timed out.
bool CloseSessionWithUser( const SteamNetworkingIdentity &identityRemote )
{
    PRINT_DEBUG("Steam_Networking_Messages::CloseSessionWithUser\n");
    return false;
}

/// Call this  when you're done talking to a user on a specific channel.  Once all
/// open channels to a user have been closed, the open session to the user will be
/// closed, and any new data from this user will trigger a SteamP2PSessionRequest_t
/// callback
bool CloseChannelWithUser( const SteamNetworkingIdentity &identityRemote, int nLocalChannel )
{
    PRINT_DEBUG("Steam_Networking_Messages::CloseChannelWithUser\n");
    return false;
}

/// Returns information about the latest state of a connection, if any, with the given peer.
/// Primarily intended for debugging purposes, but can also be used to get more detailed
/// failure information.  (See SendMessageToUser and k_nSteamNetworkingSend_AutoRestartBrokwnSession.)
///
/// Returns the value of SteamNetConnectionInfo_t::m_eState, or k_ESteamNetworkingConnectionState_None
/// if no connection exists with specified peer.  You may pass nullptr for either parameter if
/// you do not need the corresponding details.  Note that sessions time out after a while,
/// so if a connection fails, or SendMessageToUser returns SendMessageToUser, you cannot wait
/// indefinitely to obtain the reason for failure.
ESteamNetworkingConnectionState GetSessionConnectionInfo( const SteamNetworkingIdentity &identityRemote, SteamNetConnectionInfo_t *pConnectionInfo, SteamNetworkingQuickConnectionStatus *pQuickStatus )
{
    PRINT_DEBUG("Steam_Networking_Messages::GetSessionConnectionInfo\n");
    return k_ESteamNetworkingConnectionState_None;
}


void RunCallbacks()
{
}

void Callback(Common_Message *msg)
{
    if (msg->has_low_level()) {
        if (msg->low_level().type() == Low_Level::CONNECT) {
            
        }

        if (msg->low_level().type() == Low_Level::DISCONNECT) {

        }
    }
}

};