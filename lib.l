(setq defmacro (macro ('name . 'def)
  (list 'setq name (cons 'macro def))))

(defmacro defun ('name . 'def)
  (list 'setq name (cons 'lambda def)))

(defmacro while ('cond . 'code)
  (let ((while-impl (gensym))
        (while-impl-ret (gensym)))
    (list 'let (list while-impl while-impl-ret)
      (list 'defun while-impl '()
        (list 'if cond
          (list 'progn
            (list 'setq while-impl-ret (cons 'progn code))
            (list while-impl))))
      ;; run loop
      (list while-impl)))

(defun rep (fn)
  (lambda args (print (eval (cons fn args)))))
