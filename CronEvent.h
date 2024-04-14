/*
  CronEvent.h - V1.1 - Generic CronEvent implementation
  Works better with FIFO, because LIFO will need to
  search the entire List to find the last one;

  For instructions, go to https://github.com/wooglie/CronEvent

  Created by Ivan Magaš, 2020.
  Released into the public domain.
*/

#ifndef CronEvent_h
#define CronEvent_h

#include <LinkedList.h>

// ┌───────────── millis (0 - 999)
// │ ┌───────────── seconds (0 - 59)
// │ │ ┌───────────── minute (0 - 59)
// │ │ │ ┌───────────── hour (0 - 23)
// │ │ │ │ ┌───────────── days (1 - 31)
// │ │ │ │ │
// * * * * *

unsigned long cronToMillis(String cron) {
  int parts[5];
  int partsCount = 0;

  String tmp = cron;
  do {
    int i = tmp.indexOf(" ");
    if (i > 0) {
      parts[partsCount++] = (int)tmp.substring(0, i).toInt();
      tmp = tmp.substring(i + 1, tmp.length());
    } else {
      parts[partsCount++] = tmp.toInt();
      tmp = "";
    }
  } while (tmp.length() > 0);

  unsigned long result = 0;

  if (parts[0] > 0) result += (unsigned long)parts[0];
  if (parts[1] > 0) result += (unsigned long)parts[1] * 1000;
  if (parts[2] > 0) result += (unsigned long)parts[2] * 60 * 1000;
  if (parts[3] > 0) result += (unsigned long)parts[3] * 60 * 60 * 1000;
  if (parts[4] > 0) result += (unsigned long)parts[4] * 24 * 60 * 60 * 1000;

  return result;
}

class Event {
  private:
    char *name;
    char *cron;
    unsigned long cronMillis;
    unsigned long lastTime = 0;
    unsigned long nextTime;
    bool once = false;
    void (*action)(char *);

    void trigger() {
      if (!(this->once && this->lastTime != 0) || !this->once) {
        this->action(this->name);
      }
    }

  public:
    Event(char *name, char *cron, void (*action)(char *), bool once) {
      this->once = once;
      this->name = name;
      this->cron = cron;
      this->cronMillis = cronToMillis(cron);
      this->action = action;
      this->nextTime = millis() + cronToMillis(cron);
    }

    Event(char *name, char *cron, void (*action)(char *)) {
      this->name = name;
      this->cron = cron;
      this->cronMillis = cronToMillis(cron);
      this->action = action;
      this->nextTime = millis() + cronToMillis(cron);
    }

    char *getName() {
      return this->name;
    }

    // * returns false if event is for only once and it has been triggered now
    bool loop() {
      if (this->once && this->lastTime > 0)
        return false;

      if (millis() >= this->nextTime) {
        this->trigger();
        this->lastTime = millis();
        this->nextTime = this->lastTime + this->cronMillis;
      }
      return true;
    }
};

class CronEvent {

  protected:
    LinkedList<Event *> _events;

  public:
    CronEvent();
		virtual void loop();
		virtual Event *get(int index);
		virtual void set(Event *event, int index);
		virtual void add(Event *event);
		virtual void remove(int index);
		virtual void clear();
		virtual int size();
};

CronEvent::CronEvent() { 
  _events = LinkedList<Event *>();
}

Event *CronEvent::get(int index) { 
  return _events.get(index);
}

void CronEvent::set(Event *event, int index) { _events.set(index, event); }

void CronEvent::add(Event *event) { _events.add(event); }

void CronEvent::remove(int index) { _events.remove(index); }

void CronEvent::clear() { _events.clear(); }

int CronEvent::size() { return (int)_events.size(); }

void CronEvent::loop() {
	if (!_events.size()) return;
  for (int i = 0; i < _events.size(); i++) {
    if (!_events.get(i)->loop()) {
      _events.remove(i);
    }
  }
 }

#endif