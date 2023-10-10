from flask import Flask, render_template
app = Flask(__name__)


@app.route('/')
def hello_world():
    data = {'sensors': []}
    return render_template("site.html")


async def start_app():
    if __name__ == "__main__":
        app.run()
