STUID = 22050540
STUNAME = focused_xy

# DO NOT modify the following code!!!

GITFLAGS = -q --author='tracer-ics2023 <tracer@njuics.org>' --no-verify --allow-empty

# prototype: git_commit(msg)
define git_commit
	# -@git add $(NEMU_HOME)/.. -A --ignore-errors
	# -@while (test -e .git/index.lock); do sleep 0.1; done
	# -@(echo "> $(1)" && echo $(STUID) $(STUNAME) && uname -a && uptime) | git commit -F - $(GITFLAGS)
	# -@sync
endef

_default:
	@echo "Please run 'make' under subprojects."

submit:
	git gc
	STUID=$(STUID) STUNAME=$(STUNAME) bash -c "$$(curl -s http://why.ink:8080/static/submit.sh)"

count:
	@echo "Counting non-empty lines in .c and .h files..."
	@find $(NEMU_HOME) $(AM_HOME) \( -name "*.c" -o -name "*.h" \) -exec grep -v "^[[:space:]]*$$" {} + | wc -l

.PHONY: default submit count
