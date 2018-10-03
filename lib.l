(setq defun (lambda ('name . 'def)
  (set name (eval (cons 'lambda def)))))

(defun while-impl ('cond . 'code)
  (if (eval cond)
    (do
      (setq while-impl-ret (eval (cons 'do code)))
      (eval (list* 'while-impl cond code)))
    ;; else
    while-impl-ret))

(defun while ('cond . 'code)
  (let (while-impl-ret)
    (eval (list* 'while-impl cond code))))
