(setq defmacro (macro ('name . 'def)
  `(setq ,name (macro . ,def))))

(defmacro defun ('name . 'def)
  `(setq ,name (lambda . ,def)))

(defmacro while ('cond . 'code)
  (let ((while-impl (gensym))
        (while-impl-ret (gensym)))
    `(let (,while-impl ,while-impl-ret)
      (defun ,while-impl ()
        (if ,cond
          (progn (setq ,while-impl-ret (progn . ,code)) (,while-impl))
          ,while-impl-ret))
      ;; run loop
      (,while-impl))))

(defun rep (fn)
  (lambda args (print (eval (cons fn args)))))
