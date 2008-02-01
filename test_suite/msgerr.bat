(defclass THINGO 
   (is-a USER)
   (slot x (create-accessor read-write))
   (multislot y (create-accessor read-write)))
(defmessage-handler THINGO blah-1 ()
   (bind ?self 3))
(defclass A (is-a USER) (role concrete))
(make-instance a of A)
(defmessage-handler A print-args (?a ?b $?c)
	(printout t (instance-name ?self) " " ?a " " ?b 
		" and " (length ?c) " extras: " ?c crlf))
(send [a] print-args 1 2)
(send [a] print-args a b c d)
(clear) 
(defclass A (is-a USER)
  (role concrete)
  (slot foo (create-accessor read-write) (default 1))
  (slot bar (create-accessor read-write) (default 2)))
(defmessage-handler A print-all-slots ()
  (printout t ?self:foo " " ?self:bar crlf))
(make-instance a of A)
(send [a] print-all-slots)
(clear)
(defclass A (is-a USER)
  (role concrete)
  (slot bar (create-accessor read-write) (default "abc")))
(make-instance a of A)
(send [a] get-bar)
(clear)
(defclass A (is-a USER) (slot x (create-accessor read-write)))
(defclass B (is-a A) (slot y (create-accessor read-write)))
(defmessage-handler B get-y ())
(defmessage-handler B put-y ())
(defmessage-handler A get-x ())
(defmessage-handler A put-x ())
(defmessage-handler A get-y ())
(defmessage-handler A put-y ())
(defmessage-handler B get-x ())
(defmessage-handler B put-x ())
(defmessage-handler USER get-x ())
(defmessage-handler USER put-x ())
(defmessage-handler USER get-y ())
(defmessage-handler USER put-y ())
(clear)
(defclass A (is-a USER) (role concrete))
(defmessage-handler A init before ()
	(printout t "Initializing a new instance of class A..." crlf))
(make-instance a of A)
(clear)
(undefmessage-handler USER init primary)
(undefmessage-handler USER delete primary)
(undefmessage-handler USER print primary)
(defmessage-handler USER init ())
(defmessage-handler USER delete ())
(defmessage-handler USER print ())
(clear)
(defclass A (is-a USER) (role abstract))
(defclass B (is-a USER) (role abstract) (slot x (create-accessor read-write)))
(describe-class A)
(describe-class B)
(clear)
(defclass A (is-a USER) (role concrete) (slot x (create-accessor read-write) (default 3)))
(make-instance a of A)
(watch all)
(send [a] init)
(unwatch all)
(clear)
(defclass A (is-a USER)
 (role concrete)
 (slot x  (create-accessor read-write) (default 1)))
(definstances A-OBJECTS 
 (a1 of A)
 ((gensym) of A (x 65)))
(watch instances)
(reset)
(reset)
(unwatch all)
(clear)
(defclass A (is-a USER)
 (role concrete)
 (slot x (create-accessor read-write) (default 34))
 (slot y (create-accessor read-write) (default 100))
 (slot z (create-accessor read-write) (default ?NONE)))
(make-instance a of A (z 98))
(send [a] print)
(send [a] put-x 65)
(send [a] put-y abc)
(send [a] put-z "Hello world.")
(send [a] print)
(initialize-instance a)
(send [a] print)
(defclass BOGUS (is-a OBJECT) (role concrete))
(make-instance bogus of BOGUS)
(clear)
(defmessage-handler)
(defmessage-handler BOGUS)
(defmessage-handler USER 5)
(defmessage-handler USER blah bad-comment)
(defmessage-handler USER a1 "comment")
(defmessage-handler USER a2 primary "comment" ())
(defmessage-handler USER a2 around "comment" ())
(defmessage-handler USER a2 before "comment" ())
(defmessage-handler USER a2 after "comment" ())
(list-defmessage-handlers)
(clear)
(defmessage-handler USER blah ($?any ?a))
(defmessage-handler USER blah (?self ?a))
(defmessage-handler USER blah (?self:bogus ?a))
(defmessage-handler USER blah (?a)
  (bind ?self 1))
(defmessage-handler USER blah (?a)
  (dynamic-put ?a))
(defclass test (is-a USER) (role concrete))
(defmessage-handler test test-arg-cnt (?a ?b)
   (printout t "Should not see this message!" crlf))
(make-instance t of test)
(send [t] test-arg-cnt)
(send [t] test-arg-cnt 1 2 3)