#include <libgba-sprite-engine/gba_engine.h>
#include <tonc.h>

#include "../lib/LinkConnection.h"
#include "scenes/TestScene.h"
#include "utils/SceneUtils.h"

void setUpInterrupts();
void printTutorial();
static std::shared_ptr<GBAEngine> engine{new GBAEngine()};
static std::unique_ptr<TestScene> testScene{new TestScene(engine)};
LinkConnection* linkConnection = new LinkConnection();

int main() {
  setUpInterrupts();

  engine->setScene(testScene.get());

  printTutorial();

  while (true) {
    u16 keys = ~REG_KEYS & KEY_ANY;

    // enable and disable
    if ((keys & KEY_DOWN) && linkConnection->isActive()) {
      linkConnection->deactivate();
      DEBULOG("! stopped");
    }
    if ((keys & KEY_START) && !linkConnection->isActive()) {
      linkConnection->activate();
      DEBULOG("! started");
    }

    // log player id/count and important flags
    TextStream::instance().setText(
        "P" + asStr(linkConnection->linkState->currentPlayerId) + "/" +
            asStr(linkConnection->linkState->playerCount) + "-R" +
            asStr(isBitHigh(REG_SIOCNT, LINK_BIT_READY)) + "-S" +
            asStr(isBitHigh(REG_SIOCNT, LINK_BIT_START)) + "-E" +
            asStr(isBitHigh(REG_SIOCNT, LINK_BIT_ERROR)) + "-I" +
            asStr(linkConnection->linkState->_IRQFlag),
        0, 11);

    engine->update();

    VBlankIntrWait();
  }

  return 0;
}

inline void ISR_reset() {
  RegisterRamReset(RESET_REG | RESET_VRAM);
  SoftReset();
}

inline void setUpInterrupts() {
  irq_init(NULL);

  // LinkConnection
  irq_add(II_VBLANK, LINK_ISR_VBLANK);
  irq_add(II_SERIAL, LINK_ISR_SERIAL);
  irq_add(II_TIMER3, LINK_ISR_TIMER);
  irq_add(II_TIMER2, NULL);

  // A+B+START+SELECT
  REG_KEYCNT = 0b1100000000001111;
  irq_add(II_KEYPAD, ISR_reset);
}

void printTutorial() {
  DEBULOG("gba-link-connection demo");
  DEBULOG("");
  DEBULOG("START: turn on connection");
  DEBULOG("(on connection, p1 sends 999)");
  DEBULOG("");
  DEBULOG("A: send counter++ (once)");
  DEBULOG("B: send counter++ (cont)");
  DEBULOG("L: send counter++ twice (once)");
  DEBULOG("R: send counter++ twice (cont)");
  DEBULOG("SELECT: force lag (9k lines)");
  DEBULOG("DOWN: turn off connection");
  DEBULOG("");
}
