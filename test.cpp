#include <bits/stdc++.h>
using namespace std;

int main() {
	system("rm -rf test.out");
	for (int i = 1; i <= 100; ++i) {
		//system("./bin/img_char_number/recognize ./train/img/char/number/1/1.png 2>>test.out");
		system("./bin/img_char_number/recognize ~/图片/屏幕截图/2025-08-29.00:16:11.png 2>>test.out");
	}
	filebuf fb;
	istream is(&fb);
	fb.open("test.out", ios::in);
	int ans = 0;
	for (int i = 1; i <= 100; ++i) {
		int result; is >> result;
		ans += (result == 8);
	}
	cout << ans << "/100" << endl;
}
