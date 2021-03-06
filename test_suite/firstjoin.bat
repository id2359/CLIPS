(clear)                     ; Sharing
(watch activations)
(defrule foo =>)
(agenda)
(run)
(reset)
(agenda)
(defrule bar =>)
(agenda)
(reset)
(agenda)
(clear)                     ; Bug 1
(set-strategy mea)
(defrule r1 =>))
(defrule r7 (not (x)) =>)
(agenda)
(set-strategy lex)
(reset)
(agenda)
(run)
(set-strategy depth)
(clear)                     ; First CE is not
(defrule foo (not (a)) =>)
(agenda)
(assert (a))
(retract 1)
(agenda)
(reset)
(agenda)
(assert (a))
(reset)
(clear)                     ; First CE is not/logical

(defrule foo 
   (logical (not (a)))
   =>
   (assert (b)))
(watch facts)
(run)
(assert (a))
(clear)                     ; First CE is not/and
(defrule foo (not (and (a) (b))) =>)
(assert (a))
(assert (b))
(retract 2)
(clear)                     ; First CE is test
(defrule foo (test (> 4 3)) =>)
(defrule bar (test (< 4 3)) =>)
(unwatch facts)
(clear)                     ; First CE is not with test
(defrule foo (not (a)) (test (> 0 1)) =>)
(defrule bar (not (b)) (test (< 0 1)) =>)
(reset)
(clear)
(assert (a) (b))
(defrule foo (not (a)) (test (> 0 1)) =>)
(defrule bar (not (b)) (test (< 0 1)) =>)
(reset)
(defrule foo2 (not (a)) (test (> 0 1)) (not (x)) =>)
(defrule bar2 (not (b)) (test (< 0 1)) (not (x)) =>)
(clear)
(defrule bar (a) (not (and (test (< 5 3)) (b))) (c) =>)
(matches bar)
(defrule foo (test (> 3 4)) =>)
(defrule yak (test (< 3 4)) =>)
(matches foo)
(matches yak)
(clear)                     ; Incremental Reset exists/nand CE
(defrule foo (a) (b) (c) =>)
(assert (a) (b) (c) (d))
(defrule bar (exists (a) (b)) (d) =>)
(defrule yak (exists (a) (b)) (c) =>)
(defrule woz (not (and (a) (b))) (d) =>)
(unwatch activations)       ; Done
(clear)
