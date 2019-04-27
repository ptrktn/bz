;; .emacs

;;; uncomment this line to disable loading of "default.el" at startup
;; (setq inhibit-default-init t)

;; turn on font-lock mode
(when (fboundp 'global-font-lock-mode)
  (global-font-lock-mode t))

;; enable visual feedback on selections
(setq transient-mark-mode t)

;; default to better frame titles
(setq frame-title-format
      (concat  "%b - emacs@" (system-name)))

;; default to unified diffs
(setq diff-switches "-u")

;; always end a file with a newline
;(setq require-final-newline 'query)

(show-paren-mode 1) 
;;(set-background-color "antiquewhite") 
;;(set-background-color "#white") 
(if (> (display-color-cells) 20)
	(set-background-color "antiquewhite")
  (set-background-color "#white") 
)
(fset 'yes-or-no-p 'y-or-n-p) 

(setq initial-frame-alist '((top . 1) (left . 1) (width . 80) (height . 40)))
(add-to-list 'default-frame-alist '(height . 40))
(add-to-list 'default-frame-alist '(width . 80))

(setq-default tab-width 4)
(add-hook 'c-mode-hook '(lambda () (setq c-basic-offset 4)))
(add-hook 'c++-mode-hook '(lambda () (setq c-basic-offset 4)))

;; in case there is php-mode.el in ~/bin/emacs
;; (add-to-list 'load-path "~/bin/emacs")
;; (require 'php-mode)
;; (add-hook 'php-mode-user-hook 'turn-on-font-lock)

;; (autoload 'javascript-mode "javascript" nil t)
;; (add-to-list 'auto-mode-alist '("\.js\'" . javascript-mode))
(custom-set-variables
  ;; custom-set-variables was added by Custom.
  ;; If you edit it by hand, you could mess it up, so be careful.
  ;; Your init file should contain only one such instance.
  ;; If there is more than one, they won't work right.
 '(inhibit-startup-screen t))

(custom-set-faces
  ;; custom-set-faces was added by Custom.
  ;; If you edit it by hand, you could mess it up, so be careful.
  ;; Your init file should contain only one such instance.
  ;; If there is more than one, they won't work right.
 )

;;(setq ange-ftp-make-backup-files nil)
;;(setq backup-directory-alist nil)
(add-to-list 'backup-directory-alist
			 (cons "." "~/.emacs.d/backups/"))
(setq tramp-backup-directory-alist backup-directory-alist)
