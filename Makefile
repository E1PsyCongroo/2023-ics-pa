STUID = 22050540
STUNAME = focused_xy

# DO NOT modify the following code!!!

GITFLAGS = -q --author='tracer-ics2023 <tracer@njuics.org>' --no-verify --allow-empty

# prototype: git_commit(msg)
# define git_commit
#  	-@git add $(NEMU_HOME)/.. -A --ignore-errors
#  	-@while (test -e .git/index.lock); do sleep 0.1; done
#  	-@(echo "> $(1)" && echo $(STUID) $(STUNAME) && uname -a && uptime) | git commit -F - $(GITFLAGS)
#  	-@sync
# endef
define git_commit
endef

_default:
	@echo "Please run 'make' under subprojects."

submit:
	git gc
	STUID=$(STUID) STUNAME=$(STUNAME) bash -c "$$(curl -s http://why.ink:8080/static/submit.sh)"

count:
	@echo "Counting non-empty lines in .c and .h files..."
	@echo "NEMU:"
	@echo -n ".c: "
	@find $(NEMU_HOME) -name "*.c" -exec grep -v "^[[:space:]]*$$" {} + | wc -l
	@echo -n ".h: "
	@find $(NEMU_HOME) -name "*.h" -exec grep -v "^[[:space:]]*$$" {} + | wc -l
	@echo "Nanos"
	@echo -n ".c: "
	@find $(NEMU_HOME)/../nanos-lite -name "*.c" -exec grep -v "^[[:space:]]*$$" {} + | wc -l
	@echo -n ".h: "
	@find $(NEMU_HOME)/../nanos-lite -name "*.h" -exec grep -v "^[[:space:]]*$$" {} + | wc -l
	@echo "AM:"
	@echo -n ".c: "
	@find $(AM_HOME) -name "*.c" -exec grep -v "^[[:space:]]*$$" {} + | wc -l
	@echo -n ".h: "
	@find $(AM_HOME) -name "*.h" -exec grep -v "^[[:space:]]*$$" {} + | wc -l

.PHONY: default submit count
