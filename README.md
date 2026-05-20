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

## TODO

- [x] Single screens, endless scrolling, or a mix? ...single screens. Keep it simple.
- [x] Starter graphics.
- [x] Map and sprite loader.
- [ ] General physics. I think we can do 1d impulse physics on rects as usual, nothing super fancy.
- [ ] Hero.
- - [ ] Walk.
- - [ ] Jump.
- - [ ] Duck.
- - [ ] Down-jump.
- - [ ] Wall slide.
- - [ ] Wall jump.
- - [ ] Dash.
- - [ ] Floor slide: Dash while ducking.
- - [ ] Carry.
- [ ] Rest of environment.
- - [ ] Solids.
- - [ ] One-way platforms.
- - [ ] Moving platforms.
- - [ ] Hazards.
- [ ] Things to find and deliver.
- [ ] Boss. He'll offer to trade his Thing for something you can collect outside. Watermelon?
- [ ] Sound effects.
- [ ] Music.
- [ ] Itch page.
