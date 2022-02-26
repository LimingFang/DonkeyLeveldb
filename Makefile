.PHONY: format

format:
	fd '.+\.[cpp|h|cc]' . | xargs  clang-format -i --style=google