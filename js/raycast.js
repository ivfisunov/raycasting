
const TILE_SIZE = 64;
const MAP_NUM_ROWS = 11;
const MAP_NUM_COLS = 15;

const WINDOW_WIDTH = MAP_NUM_COLS * TILE_SIZE;
const WINDOW_HEIGHT = MAP_NUM_ROWS * TILE_SIZE;

const FOV_ANGLE = 60 * (Math.PI / 180);

const WALL_STRIP_WIDTH = 1;
const NUM_RAYS = WINDOW_WIDTH / WALL_STRIP_WIDTH;

const MINIMAP_SCALE_FACTOR = 0.25;

class Grid {
    constructor() {
        this.grid = [
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1],
            [1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 1, 1, 1, 1, 2, 0, 0, 0, 1, 1, 1, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
        ]
    }

    // isItWall(row, col) {
    //   if (this.grid[row][col] === 1) {
    //     return true;
    //   }
    //   return false;
    // }
    /*hasWallAt(x, y) {
      if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT) {
        return true;
      }
      var mapGridIndexX = Math.floor(x / TILE_SIZE);
      var mapGridIndexY = Math.floor(y / TILE_SIZE);
      return this.grid[mapGridIndexY][mapGridIndexX] != 0;
    }*/
    hasWallAt(x, y) {
        if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT) {
            return true;
        }

        // calculate the next position of a player
        const row = Math.floor(y / TILE_SIZE);
        const col = Math.floor(x / TILE_SIZE);
        return this.grid[row][col];
    }

    render() {
        for (let i = 0; i < MAP_NUM_ROWS; i++) {
            for (let j = 0; j < MAP_NUM_COLS; j++) {
                let tileX = j * TILE_SIZE;
                let tileY = i * TILE_SIZE;
                let tileColor = '#fff';
                if (this.grid[i][j] !== 0) {
                    tileColor = '#222';
                }
                // let tileColor = this.grid[i][j] === 1 ? '#222' : '#fff';
                stroke('#222');
                fill(tileColor);
                rect(
                    MINIMAP_SCALE_FACTOR * tileX,
                    MINIMAP_SCALE_FACTOR * tileY,
                    MINIMAP_SCALE_FACTOR * TILE_SIZE,
                    MINIMAP_SCALE_FACTOR * TILE_SIZE);
            }
        }
    }
}

class Player {
    constructor(grid) {
        this.grid = grid;
        this.x = WINDOW_WIDTH / 2;
        this.y = WINDOW_HEIGHT / 2;
        this.radius = 6;
        this.turnDirection = 0; // -1 if left, +1 if right
        this.walkDirection = 0; // -1 if back, +1 if front
        this.rotationAngle = Math.PI / 2;
        this.moveSpeed = 4.0;
        this.rotationSpeed = 4 * (Math.PI / 180);
    }

    update() {
        this.rotationAngle += this.turnDirection * this.rotationSpeed;

        let moveStep = this.walkDirection * this.moveSpeed;

        let newPlayerX = this.x + Math.cos(this.rotationAngle) * moveStep;
        let newPlayerY = this.y + Math.sin(this.rotationAngle) * moveStep;
        // if next position is not a wall, update position
        if (!grid.hasWallAt(newPlayerX, newPlayerY)) {
            this.x = newPlayerX;
            this.y = newPlayerY;
        }
    }

    render() {
        noStroke();
        fill('green');
        circle(
            MINIMAP_SCALE_FACTOR * this.x,
            MINIMAP_SCALE_FACTOR * this.y,
            MINIMAP_SCALE_FACTOR * this.radius);
        stroke('red');
        line(
            MINIMAP_SCALE_FACTOR * this.x,
            MINIMAP_SCALE_FACTOR * this.y,
            MINIMAP_SCALE_FACTOR * (this.x + Math.cos(this.rotationAngle) * 30),
            MINIMAP_SCALE_FACTOR * (this.y + Math.sin(this.rotationAngle) * 30)
        )
    }
}

class Ray {
    constructor(rayAngle) {
        this.rayAngle = normalizedAngle(rayAngle);
        this.wallHitX = 0;
        this.wallHitY = 0;
        this.distance = 0;
        this.wasHitVertical = false;

        this.isRayFacingDown = this.rayAngle > 0 && this.rayAngle < Math.PI;
        this.isRayFacingUp = !this.isRayFacingDown;
        this.isRayFacingRight = this.rayAngle < 0.5 * Math.PI || this.rayAngle > 1.5 * Math.PI;
        this.isRayFacingLeft = !this.isRayFacingRight;

        this.wallColor = '255, 255, 255';
    }

    cast() {
        let xintercept, yintercept;
        let xstep, ystep;

        /**
         * Horizontal Ray-grid interception code
         */
        let foundHorzWallHit = false;
        let horzWallHitX = 0, horzWallHitY = 0;
        // find the Y coordinate of the closest horizontal intersection
        yintercept = Math.floor(player.y / TILE_SIZE) * TILE_SIZE;
        yintercept += this.isRayFacingDown ? TILE_SIZE : 0;
        // find the X coordinate of the closest horizontal intersection
        // xintercept = player.x + (player.y - yintercept) / Math.tan(this.rayAngle); // check !!!!
        xintercept = player.x + (yintercept - player.y) / Math.tan(this.rayAngle); // check !!!!

        ystep = TILE_SIZE;
        ystep *= this.isRayFacingUp ? -1 : 1;

        xstep = TILE_SIZE / Math.tan(this.rayAngle);
        xstep *= (this.isRayFacingLeft && xstep > 0) ? -1 : 1;
        xstep *= (this.isRayFacingRight && xstep < 0) ? -1 : 1;

        let nextHorzTouchX = xintercept;
        let nextHorzTouchY = yintercept;

        // if (this.isRayFacingUp) {
        //   nextHorzTouchY--;
        // }

        while (
            nextHorzTouchX >= 0 &&
            nextHorzTouchX <= WINDOW_WIDTH &&
            nextHorzTouchY >= 0 &&
            nextHorzTouchY <= WINDOW_HEIGHT
            ) {
            const isWall = grid.hasWallAt(nextHorzTouchX, nextHorzTouchY - (this.isRayFacingUp ? 1 : 0));
            if (isWall) {
                // wall
                if (isWall === 2) {
                    this.wallColor = '255, 0, 0';
                }
                foundHorzWallHit = true;
                horzWallHitX = nextHorzTouchX;
                horzWallHitY = nextHorzTouchY;
                break;
            } else {
                nextHorzTouchX += xstep;
                nextHorzTouchY += ystep;
            }
        }

        /**
         * Vertical Ray-grid interception code
         */
        let foundVertWallHit = false;
        let vertWallHitX = 0, vertWallHitY = 0;
        // find the X coordinate of the closest vertical intersection
        xintercept = Math.floor(player.x / TILE_SIZE) * TILE_SIZE;
        xintercept += this.isRayFacingRight ? TILE_SIZE : 0;
        // find the Y coordinate of the closest vertical intersection
        // xintercept = player.x + (player.y - yintercept) / Math.tan(this.rayAngle); // check !!!!
        yintercept = player.y + (xintercept - player.x) * Math.tan(this.rayAngle); // check !!!!

        xstep = TILE_SIZE;
        xstep *= this.isRayFacingLeft ? -1 : 1;

        ystep = TILE_SIZE * Math.tan(this.rayAngle);
        ystep *= (this.isRayFacingUp && ystep > 0) ? -1 : 1;
        ystep *= (this.isRayFacingDown && ystep < 0) ? -1 : 1;

        let nextVertTouchX = xintercept;
        let nextVertTouchY = yintercept;

        // if (this.isRayFacingLeft) {
        //   nextVertTouchX--;
        // }

        while (
            nextVertTouchX >= 0 &&
            nextVertTouchX <= WINDOW_WIDTH &&
            nextVertTouchY >= 0 &&
            nextVertTouchY <= WINDOW_HEIGHT
            ) {
            const isWall = grid.hasWallAt(nextVertTouchX - (this.isRayFacingLeft ? 1 : 0), nextVertTouchY);
            if (isWall) {
                // wall
                if (isWall === 2) {
                  this.wallColor = '255, 0, 0';
                }
                foundVertWallHit = true;
                vertWallHitX = nextVertTouchX;
                vertWallHitY = nextVertTouchY;
                break;
            } else {
                nextVertTouchX += xstep;
                nextVertTouchY += ystep;
            }
        }

        let horzHitDist = foundHorzWallHit
            ? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY)
            : Number.MAX_VALUE;
        let vertHitDist = foundVertWallHit
            ? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY)
            : Number.MAX_VALUE;

        // store the smallest of the distance
        if (vertHitDist < horzHitDist) {
            this.wallHitX = vertWallHitX;
            this.wallHitY = vertWallHitY;
            this.distance = vertHitDist;
            this.wasHitVertical = true;
        } else {
            this.wallHitX = horzWallHitX;
            this.wallHitY = horzWallHitY;
            this.distance = horzHitDist;
            this.wasHitVertical = false;
        }
    }

    render() {
        stroke("rgba(255, 0, 0, 0.3)");
        line(
            MINIMAP_SCALE_FACTOR * player.x,
            MINIMAP_SCALE_FACTOR * player.y,
            MINIMAP_SCALE_FACTOR * this.wallHitX,
            MINIMAP_SCALE_FACTOR * this.wallHitY
        )
    }
}

const grid = new Grid();
const player = new Player(grid);
let rays = [];

function distanceBetweenPoints(x1, y1, x2, y2) {
    return Math.sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

function normalizedAngle(angle) {
    let normAngle = angle % (2 * Math.PI);
    if (normAngle < 0) {
        normAngle = 2 * Math.PI + normAngle;
    }
    return normAngle;
}

function keyPressed() {
    if (keyCode === UP_ARROW) {
        player.walkDirection = +1;
    } else if (keyCode === DOWN_ARROW) {
        player.walkDirection = -1;
    } else if (keyCode === LEFT_ARROW) {
        player.turnDirection = -1;
    } else if (keyCode === RIGHT_ARROW) {
        player.turnDirection = +1;
    }
}

function keyReleased() {
    if (keyCode === UP_ARROW) {
        player.walkDirection = 0;
    } else if (keyCode === DOWN_ARROW) {
        player.walkDirection = 0;
    } else if (keyCode === LEFT_ARROW) {
        player.turnDirection = 0;
    } else if (keyCode === RIGHT_ARROW) {
        player.turnDirection = 0;
    }
}

function castAllRayas() {
    // start first ray subtracting half of FOV
    let rayAngle = player.rotationAngle - (FOV_ANGLE / 2);
    // every call of this function we need to reset array of rays
    rays = [];
    for (let i = 0; i < NUM_RAYS; i++) {
        let ray = new Ray(rayAngle);
        ray.cast();
        rays.push(ray);
        rayAngle += FOV_ANGLE / NUM_RAYS;
    }
}

function render3DProjectionWalls() {
    for (let i = 0; i < NUM_RAYS; i++) {
        const ray = rays[i];

        // to reduce a fish eye effect
        const correctWallDistance = ray.distance * Math.cos(ray.rayAngle - player.rotationAngle);
        // console.log(correctWallDistance);
        // calculate the distance to the projection plane
        const distanceProjectionsPlane = (WINDOW_WIDTH / 2) / Math.tan(FOV_ANGLE / 2);

        // projected wall height
        const wallStripHeight = (TILE_SIZE / correctWallDistance) * distanceProjectionsPlane;

        const alpha = 1.0 - (correctWallDistance / 1000); // 200 / correctWallDistance;

        const color = ray.wallColor;
        fill(`rgba(${color}, ${alpha})`);
        noStroke();
        rect(
            i * WALL_STRIP_WIDTH,
            (WINDOW_HEIGHT / 2) - (wallStripHeight / 2),
            WALL_STRIP_WIDTH,
            wallStripHeight
        );
    }
}

function setup() {
    createCanvas(WINDOW_WIDTH, WINDOW_HEIGHT);
}

function update() {
    player.update();
    castAllRayas();
}

function draw() {
    clear();

    update();

    render3DProjectionWalls();
    grid.render();

    for (ray of rays) {
        ray.render();
    }

    player.render();

}

