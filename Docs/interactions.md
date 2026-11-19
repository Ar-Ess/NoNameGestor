# Interaction model

Program state is sampled **when the transcription arrives** (about 1 s after the
user spoke), before anything is cancelled. It is computed by `VoiceManager::GetState()`.

## Program states
- **IDLE**: nothing running, nothing being spoken.
- **RUNNING**: a long command is active (`reading == true`), including the gaps between nodes.
- **MID_SENTENCE**: a response is being synthesized or spoken, no long command active.
- **WAITING**: the program asked a question and expects an answer. *(Not implemented yet.)*

## User events
- **Initiating**: PTT key pressed, or the wake word recognized. Both are one activation event.
- **Talking**: the user is speaking. Only produces a UI indicator.
- **Unknown**: the phrase matches no action.
- **Known**: the phrase matches an action.
- **Stop**: the "stop" action.
- Noise and Whisper hallucinations are discarded by the Interpreter and never enter this chart.

## Matrix

| Program / User | Initiating | Talking | Unknown | Known | Stop |
|---|---|---|---|---|---|
| IDLE | Open mic | Show indicator | "Sorry, I couldn't understand you." | Run it | "Nothing is running." |
| RUNNING | Nothing (no barge-in) | Nothing | Ignore, log only | Cancel, run new, no extra reply | Cancel, "Okay." |
| MID_SENTENCE | Nothing | Nothing | Ignore, log only | Cut voice, run new, no extra reply | Cut voice, "Okay." (action stays) |
| WAITING | Nothing | Nothing | Re-ask + "Say stop to cancel." | Answer if it fits, otherwise abandon question and run | Cancel question, "Okay." |

Wake-word mode: without the word, everything is ignored silently while the window is closed.
A bare wake word answers "Tell me!" only in IDLE. In RUNNING / MID_SENTENCE it is ignored.

## Rules
1. A recognized command always wins over whatever is in progress (D1). No "stop first" ritual.
2. "Stop" cuts the voice and cancels long commands. It never undoes an action already executed; "undo" does.
3. Answer vs. command (WAITING): "stop" always cancels, then the question's own parser
   gets the first chance (it declares an expected type: amount, name/ordinal, yes/no),
   then any known command abandons the question, otherwise re-ask.
4. Destructive confirmations need an explicit yes, and expire after about 60 s.
   Plain questions never expire.
5. Failed answers never cancel automatically; re-ask with "Say stop to cancel."
6. No barge-in: speech does not cut the voice. Only "stop" or a new known command does.
7. Modification responses state the result first and briefly.

## Modifications
- Confirmation: user preference (always / just do it). Destructive commands always confirm.
- Confirmation repeats the numbers: "Add 50 euros to Rent. Confirm?"
- Undo is the journal (inverse of each operation). A throttled backup via a host hook
  (`OnBeforeModify`) is the safety net.

## Parked
- Exposing state to the host: `IsMicOpen()`, `IsListening()`, `IsSpeaking()`, `GetState()`.
- Earcons and mic indicators; re-ask timer; strict answer mode; confirmation preference.
- ConversationLog (user-facing) and a separate debug log.
- LLM resolver: it only changes how a phrase becomes an Intent (and could assist the
  answer parser). The chart does not change.