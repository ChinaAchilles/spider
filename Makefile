spider:spider.cpp
	g++ $^ -o $@ -lcurl -lboost_regex 
.PHONY:clean
clean:
	rm spider
