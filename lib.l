;;; Broken, we need macros
;;; does not capture scope
(setq defun (lambda ('name . 'def)
  (set name (eval (cons 'lambda def)))))

;;; Also broken
;;; code is not run in its own scope
(defun while ('cond . 'code)
  (let ((while-body (eval (list* 'lambda () code)))
        while-impl
        while-impl-ret)
    (setq while-impl (lambda ('cond)
      (if (eval cond)
        (progn
          (setq while-impl-ret (while-body))
          (eval (list 'while-impl cond)))
        ;; else
        while-impl-ret)))
    ;; run loop
    (eval (list 'while-impl cond))))

(defun rep (fn)
  (lambda 'args (eval (cons fn . args))))
