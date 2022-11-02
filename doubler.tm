 
 
# podwaja kazda litere poza w

num-tapes: 2          # mendatory - number of tapes
input-alphabet: x y w         # mendatory - letters allowed to occur on input

(start) x _ (copy) x x - >
(start) y _ (copy) y y - >
(start) w _ (start) w w > >
(copy) x _ (start) x x > >
(copy) y _ (start) y y > >
(copy) w _ (start) w w > >
(start) _ _ (accept) _ _ - -