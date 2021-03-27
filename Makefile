OBJDIR = bin
DEPDIR = deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

src = $(wildcard src/*.cc)
obj = $(src:src/%.cc=$(OBJDIR)/%.o)
DEPFILES = $(src:src/%.cc=$(DEPDIR)/%.d)


# if boost is correctly installed, no need to add pathes to
# include and lib files: -I$(BOOST_DIR)/include -L$(BOOST_DIR)/lib

CXXFLAGS = -std=c++2a -Wall
LDFLAGS = -lboost_program_options

OPTFLAGS = -O3 -march=native
DBGFLAGS = -g


MODE != cat .make-switch 2> /dev/null
ifeq ($(MODE),)
	MODE := release
endif

ifeq ($(MODE), release)
	CXXFLAGS += $(OPTFLAGS)
	LDFLAGS += $(OPTFLAGS)
else
	CXXFLAGS += $(DBGFLAGS)
	LDFLAGS += $(DBGFLAGS)
endif


CXX = g++ $(CXXFLAGS)
LINKER= g++


# Add -static flag to make the exe portable
flood-fill: $(obj)
	$(LINKER) $^ -o $@ $(LDFLAGS)

release:
	@grep -q $@ .make-switch 2> /dev/null \
		|| { echo $@ > .make-switch; \
				 rm -rf $(OBJDIR)/* $(DEPDIR)/*; }
debug:
	@grep -q $@ .make-switch 2> /dev/null \
		|| { echo $@ > .make-switch; \
				 rm -rf $(OBJDIR)/* $(DEPDIR)/*; }


$(OBJDIR)/%.o: src/%.cc $(DEPDIR)/%.d | $(DEPDIR) $(OBJDIR) 
	$(CXX) $(DEPFLAGS) -c $< -o $@

$(DEPDIR):
	mkdir -p $@

$(OBJDIR):
	mkdir -p $@

$(DEPFILES):

include $(DEPFILES)


.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(DEPDIR) .make-switch
