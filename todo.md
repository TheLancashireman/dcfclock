# Still to do ...

* Synchronize the timekeeper task in settime(). At the moment, the time could be up to 1 second ahead.
* Check the operation of ReadTime(). Is an interrupt lock needed?
* Clean up the source code. Check for consistent style.
* Add the interface to the DCF receiver.
* Improve the software DCF decoder; the current version doesn't reject poor signals (out-of-range timing,
  data errors). Add some plausibility checks.
