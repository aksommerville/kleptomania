# Kleptomania

Requires [Egg v2](https://github.com/aksommerville/egg2) to build.

For [Uplifting Jam #8](https://itch.io/jam/uplifting-game-jam-315-in-cash-prizes), May 2026.
Theme: "CASTLE".

The vampire has stolen treasures from the villagers. Steal them back!
Platformer. A to jump, B to dash, drop thing, or talk. Deliberately same key for those: You can't dash while carrying something.
Dash recharges on any collision. So you can dash along a ceiling if you're careful about it.
Make it fully deterministic and satisfying for speed running.
Touch a Thing to pick it up, carry it into a Villager to deliver it, or press B to drop it.

## Agenda

Starting 18 May and due 1 June 13:00. 1 June is a Monday, so be sure we're submitted Sunday night at the latest.
2026-05-18 M 
2026-05-19 T 
2026-05-20 W 
2026-05-21 R 
2026-05-22 F Have something respectably playable by EOD.
2026-05-23 S 
2026-05-24 U Monday is Memorial Day! We get a freebie!
2026-05-25 M Aim to be complete by EOD, plenty of time to decide what more it needs then.
2026-05-26 T 
2026-05-27 W 
2026-05-28 R Ideally submit today. Keep the second weekend free for overflow if needed, and otherwise Bellacopia.
2026-05-29 F 
2026-05-30 S 
2026-05-31 U Emergency wrap-up only. Finish and submit before EOD.
2026-06-01 M Submissions close 13:00.

## Layout and Narrative

- Start with one villager.
- After satisfying the one, more appear. Say 4?
- After satisfying those, one more appears: Little girl with a missing sock.
- When little girl appears, you also get the vampire's key. It's for carrying, like treasures.
- Go to the vampire's inner sanctum. He tells you to bring a watermelon.
- Fetch watermelon from outside. These have been available all along.
- Trade watermelon for sock, return the sock, and you win!

- Start outside. One screen right is the moat. Dash required to cross.
- You can scale the castle from outside and see the bell, but can't reach anything this way.
- Straight up from the first inner room to climb the bell tower.
- Straight down from the first room to reach the inner sanctum.
- Fill the world with optional challenges. Lollipops to collect.
- - Bell tower
- - Cloud city. Get there by scaling the tower from outside.

2026-05-25T14:48: I think the main quest is at least physically final now. Can clear in 1:55, but that's a stretch.
...and 2:38, with the bell rung. Could do better there I think.

## TODO

- [x] !!! Only after completing all the deliveries, on navigating from start to castle's antechamber, the drawbridge and top of water go sky-colored during the transition. The hell?
- - ...i'd missed a graf_set_image() in the drawbridge's render hook. Why it should only happen after the flags are set will remain a mystery.
- [x] Single screens, endless scrolling, or a mix? ...single screens. Keep it simple.
- [x] Starter graphics.
- [x] Map and sprite loader.
- [x] General physics. I think we can do 1d impulse physics on rects as usual, nothing super fancy.
- [x] Hero.
- - [x] Walk.
- - [x] Jump.
- - [x] Duck.
- - [x] Down-jump.
- - [x] Wall slide.
- - [x] Wall jump.
- - [x] Dash.
- - [x] Floor slide: Dash while ducking. ...There's a nice default, not sure we need anything else here.
- - [x] Carry.
- - [x] Death: Respawn instantly wherever you entered the room.
- [ ] Properer graphics.
- [x] Rest of environment.
- - [x] Solids.
- - [x] One-way platforms.
- - [x] Moving platforms.
- - [x] Hazards.
- [x] Need some kind of mitigation against navigating up when the jump is about to crest; you fall right back down.
- - We read nearest-neighbor for physics purposes, so actually there's an easy way to do this. Just always use a double-oneway around vertical neighbors.
- - Just need one mitigation at OOB detect: When going up, only go if her feet have cleared the map, instead of center.
- - UPDATE: Nix the OOB repeat for oneways, and require toes past (h-1) for an upward transition. Much neater, one doesn't even notice that any mitigation is happening.
- - Just need to ensure that such passages always have a oneway at the bottom of the top map, and the launchpad on bottom map is at row <=2.
- [x] Dialogue. Try fully passive dialogue in word bubbles. ...with villagers made, i'm not convinced that we're going to need words at all.
- [x] Things to find and deliver.
- [x] Speed clock.
- [x] Vampire.
- [ ] Vampire dialogue.
- [x] Ending.
- [ ] Hello modal.
- [ ] Pause modal, to restart or return to menu.
- [x] Add an explicit "safe point" POI command. Specifically for the room with vertical platforms, where you enter from below. To override `g.(safex,safey)` manually.
- [x] World map.
- - Start outside, with ample room for villagers.
- - Require a dash to get into the castle. A moat, perhaps?
- - [x] Optional challenge zone if you go straight up from the first castle room. Climb the bell tower!
- - - X Dash across the ceiling.
- - - X Climb wall with 1-meter protrusions.
- - - Catch a wall with a 5-meter elevated gap, it's just barely possible. ...but there's no situation where you can walljump but not dash, is there?
- - - Wall-jump midair off a thing that you just dropped. Wasn't supposed to be possible but now it is, and it's so cool I'm leaving it in.
- - - X Dash sideways repeatedly into a horizontal platform. Would that work? ...hell yeah it would
- - - Ceiling-dash on the bottom of a horizontal platform. ...I don't think this is possible in any case where dashing into the side wouldn't also work (and the side is much easier).
- [x] Sound effects.
- [x] Music.
- [ ] Enter the Konami code at any time to change your hat.
- [ ] Persist high score, and show at Hello.
- [ ] Itch page.
