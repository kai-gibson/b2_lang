type CounterMessage =
  | Increment(Int32)
  | Get(ReplySender(Int32))
  | Stop

func counter_run(message_receiver: MailboxReceiver(CounterMessage))
  count = 0
  loop
    case message_receiver.receive()
    of .Increment(amount) do set count += amount
    of .Get(reply_sender) do reply_sender.send(count)
    of .Stop              do return
    end
  end
end

func main()
  counter_mailbox_sender = spawn(counter_run)   // returns MailboxSender(CounterMessage)

  counter_mailbox_sender.send(.Increment(5))
  counter_mailbox_sender.send(.Increment(2))

  reply_sender, reply_receiver = reply_pair(Int32)
  counter_mailbox_sender.send(.Get(reply_sender))   // reply_sender moves into the message

  count_total = reply_receiver.receive()            // parks until the counter replies
  print(count_total)                                // 7

  counter_mailbox_sender.send(.Stop)
end
