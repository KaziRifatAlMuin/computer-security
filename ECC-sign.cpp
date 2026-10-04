#include <iostream>
using namespace std;

const int p = 17;
const int a = 2;
const int n = 19;

struct Point {
    int x, y;
    bool inf;
};

int mod(int x) {
    return (x % p + p) % p;
}

int modN(int x) {
    return (x % n + n) % n;
}

int inv(int x, int m) {
    x = (x % m + m) % m;

    for (int i = 1; i < m; i++)
        if ((x * i) % m == 1)
            return i;

    return -1;
}

Point add(Point A, Point B) {

    if (A.inf) return B;
    if (B.inf) return A;

    if (A.x == B.x && A.y != B.y)
        return {0, 0, true};

    int m;

    if (A.x == B.x)
        m = mod((3 * A.x * A.x + a) * inv(2 * A.y, p));
    else
        m = mod((B.y - A.y) * inv(B.x - A.x, p));

    int x = mod(m * m - A.x - B.x);
    int y = mod(m * (A.x - x) - A.y);

    return {x, y, false};
}

Point mul(int k, Point A) {

    Point R = {0, 0, true};

    while (k) {

        if (k & 1)
            R = add(R, A);

        A = add(A, A);
        k >>= 1;
    }

    return R;
}


// ---------------- EC-ElGamal ----------------

Point toPoint(int m, Point G) {
    return mul(modN(m), G);
}

int toMsg(Point M, Point G) {

    if (M.inf)
        return 0;

    for (int i = 1; i < n; i++) {

        Point T = mul(i, G);

        if (T.x == M.x && T.y == M.y)
            return i;
    }

    return -1;
}

struct Cipher {
    Point c1, c2;
};

Cipher encrypt(int m, Point G, Point Q, int k) {

    return {
        mul(k, G),
        add(toPoint(m, G), mul(k, Q))
    };
}

int decrypt(Cipher C, int d, Point G) {

    Point T = mul(d, C.c1);

    if (!T.inf)
        T.y = mod(-T.y);

    return toMsg(add(C.c2, T), G);
}

Cipher homo(Cipher A, Cipher B) {

    return {
        add(A.c1, B.c1),
        add(A.c2, B.c2)
    };
}


// ---------------- ECDSA ----------------

struct Signature {
    int r, s;
};

Signature sign(int hash, int d, int k, Point G) {

    Point R = mul(k, G);

    int r = modN(R.x);
    int s = modN(inv(k, n) * (hash + r * d));

    return {r, s};
}

bool verify(int hash, Signature sig, int d, Point G) {

    Point Q = mul(d, G);

    int w = inv(sig.s, n);
    int u1 = modN(hash * w);
    int u2 = modN(sig.r * w);

    Point R = add(
        mul(u1, G),
        mul(u2, Q)
    );

    return !R.inf && modN(R.x) == sig.r;
}


int main() {

    Point G = {5, 1, false};

    int privateKey = 3;
    Point publicKey = mul(privateKey, G);


    // ---------- Encryption ----------

    int m1, m2;

    cout << "Enter two messages (0-18): ";
    cin >> m1 >> m2;

    Cipher C1 = encrypt(m1, G, publicKey, 2);
    Cipher C2 = encrypt(m2, G, publicKey, 4);

    cout << "\nDecrypted M1 = "
         << decrypt(C1, privateKey, G) << endl;

    cout << "Decrypted M2 = "
         << decrypt(C2, privateKey, G) << endl;


    // ---------- Homomorphic Addition ----------

    int sum = decrypt(
        homo(C1, C2),
        privateKey,
        G
    );

    cout << "Homomorphic Sum = "
         << sum << endl;

    cout << "Expected Sum = "
         << (m1 + m2) % n << endl;

    if (sum == (m1 + m2) % n)
        cout << "Homomorphic Addition: SUCCESS\n";
    else
        cout << "Homomorphic Addition: FAILED\n";


    // ---------- ECDSA Sign & Verify ----------

    int hash = 8;
    int k = 7;

    Signature sig =
        sign(hash, privateKey, k, G);

    cout << "\nSignature r = "
         << sig.r << endl;

    cout << "Signature s = "
         << sig.s << endl;

    if (verify(hash, sig, privateKey, G))
        cout << "Signature: VALID\n";
    else
        cout << "Signature: INVALID\n";

    return 0;
}