# Introduction #

The message system provides:
  1. Message sending from A to a specific listener B.
  1. Multicasts: Message listeners can listen to "newsgroups" (for example mouse input events) and get informed when something happens. Senders can multicast message to certain newsgroups.
  1. Concurrency support: Message listeners specify when (inside what thread) they get their messages. For example if Thread A sends or multicasts a message which is received by a listener that runs in Thread B, the message is delivered asynchronously when Thread B processes messages.

# Writing a message listener #

First, you have to choose the right message listener type depending on what parts of the engine the listener will access in its ReceiveMessage method.
  * Derive from SynchronizedMessageListener if the listener needs access to all parts of the engine. Obviously this is bad for performance because all other threads are stopped during message processing.
  * Derive from ViewMessageListener if the listener only needs access to Ogre and parts of the engine that operate inside the rendering thread.
  * Derive from PhysicsMessageListener if the listener only needs access to PhysX and parts of the engine that operate in the physics / movement thread.
  * Derive from IndependantMessageListener if the listener doesn't need any access to other parts of the engine (except for the message system of course).

Keep in mind, it is for example possible to use Ogre directly from inside the ReceiveMessage method of an IndependantMessageListener, but it will likely produce concurrency conflicts and crashes. So please don't do things like that.